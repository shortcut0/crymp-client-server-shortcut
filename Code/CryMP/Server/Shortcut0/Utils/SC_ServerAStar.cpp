// GAME
#include "CryMP/Common/Executor.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

// Server
#include "..\SC_Server.h"
#include "SC_ServerAStar.h"

// Definition of the static member
std::unordered_map<NodePtr, std::unordered_map<NodePtr, NodeList>> SC_AStar::cachedPaths;
const float INF = std::numeric_limits<float>::infinity();

//---------------------------------------------------------------------------------------
SC_AStar::SC_AStar()
{
    m_TimerLastPath = SC_ServerTimer(1.f);
    m_CallsPerSecond = 0;
}

//---------------------------------------------------------------------------------------
SC_AStar::~SC_AStar()
{
}


//---------------------------------------------------------------------------------------
int SC_AStar::FindPath(const char* handle, SmartScriptTable start_pos, SmartScriptTable goal_pos, SmartScriptTable node_list, bool direct, HSCRIPTFUNCTION callback) {

    if (!direct)
    {
        std::unique_ptr<SC_AStar> task = std::make_unique<SC_AStar>();
        if (!task->GetPath(handle, start_pos, goal_pos, node_list))
        {
            task.reset();
            ReleaseFunc(callback);
            return 0;
        }
        if (callback != 0)
            task->lua_callback = callback;

        gSC_Server->GetExecutor()->AddTask(std::move(task));
    }
    else
    {
        if (!GetPath(handle, start_pos, goal_pos, node_list))
        {
            ReleaseFunc(callback);
            return 0;
        }

        if (callback != 0)
            this->lua_callback = callback;

        this->output = path(this->start, this->goal, this->navmesh);
        Callback();
    }

    return 1;
}

//---------------------------------------------------------------------------------------
bool SC_AStar::ResolveResults(NodeList& result, SmartScriptTable& resolve) {

    IScriptSystem* pSS = gEnv->pScriptSystem;

    // -------------------------
    // Result Table
    SmartScriptTable lua_result(pSS);

    // Loop
    int node_counter = 1; // lua is 1-based index
    for (const auto& node : result) {

        // New Element
        SmartScriptTable lua_node(pSS);

        // Element.Pos
        SmartScriptTable lua_node_pos(pSS);
        lua_node_pos->SetValue("x", node->x);
        lua_node_pos->SetValue("y", node->y);
        lua_node_pos->SetValue("z", node->z);
        lua_node->SetValue("pos", lua_node_pos);

        // Element.XYZ
        lua_node->SetValue("x", node->x);
        lua_node->SetValue("y", node->y);
        lua_node->SetValue("z", node->z);

        // Element.Id
        lua_node->SetValue("id", node->id);

        // Element.links
        SmartScriptTable lua_links = gEnv->pScriptSystem->CreateTable();
        int link_counter = 1;
        for (const auto& link : node->links) {


            SmartScriptTable lua_node_link(pSS);
            lua_node_link->SetValue("id", link.first);
            lua_node_link->SetValue("linked", link.second);

            lua_links->SetAt(link_counter, lua_node_link);
            link_counter++;
        }
        lua_node->SetValue("links", lua_links);

        // Insert node
        lua_result->SetAt(node_counter, lua_node);
        node_counter++;
    }

    // Pass along
    resolve = *lua_result;
    return true;
}

//---------------------------------------------------------------------------------------
// Converts a lua node into a C++ NodePtr
bool SC_AStar::ResolveNode(SmartScriptTable lua_node, NodePtr& resolve)
{

    // ----------------------
    std::unordered_map<int, bool> node_links;
    float node_x = -1;
    float node_y = -1;
    float node_z = -1;
    int node_id = -1;

    // ----------------------
    // Position
    lua_node->GetValue("x", node_x);
    lua_node->GetValue("y", node_y);
    lua_node->GetValue("z", node_z);

    // Alternative
    SmartScriptTable lua_node_pos;
    if (lua_node->GetValue("pos", lua_node_pos))
    {
        lua_node_pos->GetValue("x", node_x);
        lua_node_pos->GetValue("y", node_y);
        lua_node_pos->GetValue("z", node_z);
    }

    // ----------------------
    // Node ID
    lua_node->GetValue("id", node_id);
    //const char* str = "";
    //gServer->GetEvents()->Get("table.size", str, *lua_node);
    //gServer->GetEvents()->Call("Debug", "C++ DEBUG>>", str);


    // ----------------------
    // Links
    SmartScriptTable lua_links;

    if (lua_node->GetValue("links", lua_links))
    {
        IScriptTable::Iterator it = lua_links->BeginIteration();
        while (lua_links->MoveNext(it))
        {
            if (it.value.type == ANY_TTABLE)
            {
                SmartScriptTable node_link = it.value.table;
                int link_id = -1;
                bool is_linked = false;

                if (node_link->GetValue("id", link_id) && node_link->GetValue("linked", is_linked))
                {
                    //CryLogAlways("Node[%04d].Links[%03d] = %s", node_id, link_id, (is_linked ? "true" : "false"));
                    node_links[link_id] = is_linked;
                }
            }
        }
        lua_node->EndIteration(it);

    }

    // ----------------------
    // Check
    float invalid = -1;
    if (node_x == invalid || node_y == invalid || node_z == invalid || node_id == (int)invalid)
    {
        // gServer->Log("node not resolved.. id=%d", node_id);
        return false;
    }

    // ----------------------
    // Pass to resolver
    resolve->x = node_x;
    resolve->y = node_y;
    resolve->z = node_z;
    resolve->id = node_id;
    resolve->links = node_links;

    // ----------------------
    return true;
}

//---------------------------------------------------------------------------------------
bool SC_AStar::GetPath(const char* handle, SmartScriptTable t_start, SmartScriptTable t_goal, SmartScriptTable node_list)
{

    // -------------------------
    // Start And Goal
    NodePtr start = std::make_shared<Node>();
    NodePtr goal = std::make_shared<Node>();

    // Resolve
    if (!ResolveNode(t_start, start) || !ResolveNode(t_goal, goal))
        return false;

    // Cache
    if (g_pSC_ServerCVars->astar_use_cache)
    {
        /*NodeList pCache = SC_AStarCache1234::pCachedNavmesh;
        if (pCache.size() > 0)
        {

            this->handle = handle;
            this->navmesh = pCache;
            navmesh.push_back(start); // Duplicates the node, but it's fine.
            navmesh.push_back(goal); // Duplicates the node, but it's fine.

            this->start = start;
            this->goal = goal;
            return;
        }*/
    }

    // -------------------------
    // Resolve nodes
    NodeList navmesh;
    IScriptTable::Iterator it = node_list->BeginIteration();
    while (node_list->MoveNext(it))
    {
        if (SmartScriptTable lua_node = it.value.table)
        {
            NodePtr newNode = std::make_shared<Node>();
            // gServer->Log("node node node .....");
            if (ResolveNode(lua_node, newNode))
            {
                // We need to insert the nodes with the start and goal pointers!
                if (newNode->id == goal->id)
                {
                    // gServer->Log("added goal id");
                    navmesh.push_back(goal);
                }

                else if (newNode->id == start->id)
                {
                    // gServer->Log("added start id");
                    navmesh.push_back(start);
                }

                else
                    navmesh.push_back(newNode);
            }
        }

    };
    node_list->EndIteration(it);


    // -------------------------
    // Update values
    this->start = start;
    this->goal = goal;
    this->navmesh = navmesh;
    this->handle = handle;

    // -------------------------
    // Caching
    // pCachedNavmesh = navmesh;
    if (g_pSC_ServerCVars->astar_use_cache)
    {
        //SC_AStarCache1234::pCachedNavmesh = navmesh;
    }

    return true;
}

//---------------------------------------------------------------------------------------
float SC_AStar::distance(float x1, float y1, float x2, float y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

//---------------------------------------------------------------------------------------
float SC_AStar::dist_between(const NodePtr& nodeA, const NodePtr& nodeB) {
    return distance(nodeA->x, nodeA->y, nodeB->x, nodeB->y);
}

//---------------------------------------------------------------------------------------
float SC_AStar::heuristic_cost_estimate(const NodePtr& nodeA, const NodePtr& nodeB) {
    return distance(nodeA->x, nodeA->y, nodeB->x, nodeB->y);
}

//---------------------------------------------------------------------------------------
bool SC_AStar::is_valid_node(const NodePtr& node, const NodePtr& neighbor) {

    auto it = node->links.find(neighbor->id);
    bool valid = it != node->links.end() && it->second;
    return valid;
}

//---------------------------------------------------------------------------------------
NodePtr SC_AStar::lowest_f_score(const NodeList& set, const NodeScoreMap& f_score) {
    float lowest = INF;
    NodePtr bestNode = nullptr;

    for (const auto& node : set) {
        float score = f_score.at(node);
        if (score < lowest) {
            lowest = score;
            bestNode = node;
        }
    }

    return bestNode;
}

//---------------------------------------------------------------------------------------
NodeList SC_AStar::neighbor_nodes(const NodePtr& node, const NodeList& nodes) {
    NodeList neighbors;
    for (const auto& n : nodes) {
        if (node->links.find(n->id) != node->links.end() && node->links.at(n->id)) {
            neighbors.push_back(n);
        }
    }
    return neighbors;
}


//---------------------------------------------------------------------------------------
bool SC_AStar::not_in(const NodeList& set, const NodePtr& theNode) {
    return std::find(set.begin(), set.end(), theNode) == set.end();
}

//---------------------------------------------------------------------------------------
void SC_AStar::remove_node(NodeList& set, const NodePtr& theNode) {
    auto it = std::remove(set.begin(), set.end(), theNode);
    if (it != set.end()) {
        set.erase(it, set.end());
    }
}

//---------------------------------------------------------------------------------------
NodeList SC_AStar::unwind_path(NodeList& flat_path, const NodeMap& map, const NodePtr& current_node) {
    auto it = map.find(current_node);
    if (it != map.end()) {
        flat_path.insert(flat_path.begin(), it->second);
        return unwind_path(flat_path, map, it->second);
    }
    else {
        return flat_path;
    }
}
//---------------------------------------------------------------------------------------

NodeList SC_AStar::a_star(const NodePtr& start, const NodePtr& goal, const NodeList& nodes, std::function<bool(const NodePtr&, const NodePtr&)> valid_node_func) {

    NodeList closed_set;
    NodeList open_set = { start };
    NodeMap came_from;

    NodeScoreMap g_score;
    NodeScoreMap f_score;
    g_score[start] = 0;
    f_score[start] = g_score[start] + heuristic_cost_estimate(start, goal);

    while (!open_set.empty()) {
        NodePtr current = lowest_f_score(open_set, f_score);

        if (current->id == goal->id) {
            NodeList path;
            path = unwind_path(path, came_from, goal);
            path.push_back(goal);
            return path;
        }

        remove_node(open_set, current);
        closed_set.push_back(current);

        NodeList neighbors = neighbor_nodes(current, nodes);
        for (const auto& neighbor : neighbors) {
            if (not_in(closed_set, neighbor) && valid_node_func(current, neighbor)) {
                float tentative_g_score = g_score[current] + dist_between(current, neighbor);

                if (not_in(open_set, neighbor) || tentative_g_score < g_score[neighbor]) {
                    came_from[neighbor] = current;


                    g_score[neighbor] = tentative_g_score;
                    f_score[neighbor] = g_score[neighbor] + heuristic_cost_estimate(neighbor, goal);

                    if (not_in(open_set, neighbor)) {
                        open_set.push_back(neighbor);
                    }
                }
            }
        }
    }

    return NodeList(); // no valid path
}
//---------------------------------------------------------------------------------------
NodeList SC_AStar::path(const NodePtr& start, const NodePtr& goal, const NodeList& nodes, bool ignore_cache, std::function<bool(const NodePtr&, const NodePtr&)> valid_node_func) {
    if (!cachedPaths.count(start)) {
        cachedPaths[start] = {};
    }
    else if (cachedPaths[start].count(goal) && !ignore_cache) {
        return cachedPaths[start][goal];
    }

    NodeList res_path = a_star(start, goal, nodes, valid_node_func);
    if (!cachedPaths[start].count(goal) && !ignore_cache) {
        cachedPaths[start][goal] = res_path;
    }

    return res_path;
}

//---------------------------------------------------------------------------------------
void SC_AStar::clear_cached_paths() {
    cachedPaths.clear();
}

//---------------------------------------------------------------------------------------
int SC_AStar::astar_test_thread1()
{
    return 0;
};

//---------------------------------------------------------------------------------------
int SC_AStar::astar_test_thread() {
    // Example usage
    auto start = std::make_shared<Node>(Node{ 1, 0, 0, 0, {} });
    auto goal = std::make_shared<Node>(Node{ 2, 5, 5, 0, {} });
    auto node1 = std::make_shared<Node>(Node{ 3, 1, 2, 0, {} });
    auto node2 = std::make_shared<Node>(Node{ 4, 3, 4, 0, {} });

    // Define links
    start->links = { {3, true}, {4, false} };
    goal->links = { {1, true} };
    node1->links = { {1, true}, {2, true}, {4, true} };
    node2->links = { {1, true}, {3, true} };

    NodeList nodes = { start, goal, node1, node2 };
    NodeList path = SC_AStar::path(start, goal, nodes);
    for (const auto& node : path) {
        CryLogAlways("Node ID: %d, Coordinates: (%f, %f, %f)\n", node->id, node->x, node->y, node->z);
    }

    return 0;
}
