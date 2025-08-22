#pragma once

#include "CryMP/Common/Executor.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

#include <cmath>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <cstdio>

// def

#ifndef SC_ASTAR_H
#define SC_ASTAR_H

// Define constants and types


struct Node {
    int id;
    float x, y, z;
    std::unordered_map<int, bool> links;
};

using NodePtr = std::shared_ptr<Node>;
using NodeList = std::vector<NodePtr>;
using NodeMap = std::unordered_map<NodePtr, NodePtr>;
using NodeScoreMap = std::unordered_map<NodePtr, float>;
/*
namespace AStarCache1234
{
    NodeList pCachedNavmesh;
}
*/
class SC_AStar : public IExecutorTask {
public:


    SC_AStar();
    virtual ~SC_AStar();

    ///////////////////////////////////////////////////////////////

    int astar_test_thread1();
    int astar_test_thread();

    ///////////////////////////////////////////////////////////////

    // IN:
    NodePtr start; // Start pos
    NodePtr goal; // Goal pos
    NodeList navmesh; // Node map
    const char* handle; // Handle
    float start_time; // timer for statistics
    NodeList input; // unused
    HSCRIPTFUNCTION lua_callback; // lua callback

    // OUT:
    NodeList output; // path

    ///////////////////////////////////////////////////////////////

    bool ResolveResults(NodeList& result, SmartScriptTable& resolve);
    bool ResolveNode(SmartScriptTable node, NodePtr& resolve);
    bool GetPath(const char* handle, SmartScriptTable t_start, SmartScriptTable t_goal, SmartScriptTable node_list);
    int FindPath(const char* handle, SmartScriptTable start, SmartScriptTable goal, SmartScriptTable nodes, bool direct = false, HSCRIPTFUNCTION callback = NULL);


    void Execute() override
    {
        // runs in worker thread
        this->output = SC_AStar::path(this->start, this->goal, this->navmesh);
    }

    void Callback() override
    {

        SmartScriptTable lua_result(gEnv->pScriptSystem);
        ResolveResults(this->output, lua_result);

        HSCRIPTFUNCTION callback_func = this->lua_callback;
        if (callback_func != 0) {
            Script::Call(gEnv->pScriptSystem, callback_func, this->handle, *lua_result);
            ReleaseFunc(callback_func);
        }
    }


private:

    ///////////////////////////////////////////////////////////////
    static float distance(float x1, float y1, float x2, float y2);
    static float dist_between(const NodePtr& nodeA, const NodePtr& nodeB);
    static float heuristic_cost_estimate(const NodePtr& nodeA, const NodePtr& nodeB);
    static bool is_valid_node(const NodePtr& node, const NodePtr& neighbor);
    static NodePtr lowest_f_score(const NodeList& set, const NodeScoreMap& f_score);
    static NodeList neighbor_nodes(const NodePtr& theNode, const NodeList& nodes);
    static bool not_in(const NodeList& set, const NodePtr& theNode);
    static void remove_node(NodeList& set, const NodePtr& theNode);
    static NodeList unwind_path(NodeList& flat_path, const NodeMap& map, const NodePtr& current_node);
    static NodeList a_star(const NodePtr& start, const NodePtr& goal, const NodeList& nodes, std::function<bool(const NodePtr&, const NodePtr&)> valid_node_func = is_valid_node);
    static NodeList path(const NodePtr& start, const NodePtr& goal, const NodeList& nodes, bool ignore_cache = false, std::function<bool(const NodePtr&, const NodePtr&)> valid_node_func = is_valid_node);
    static void clear_cached_paths();


    void ReleaseFunc(HSCRIPTFUNCTION pFunc = 0)
    {
        if (pFunc != 0)
            gEnv->pScriptSystem->ReleaseFunc(pFunc);
    }

    SC_ServerTimer m_TimerLastPath;
    int m_CallsPerSecond;

    static std::unordered_map<NodePtr, std::unordered_map<NodePtr, NodeList>> cachedPaths;
};

#endif // SC_ASTAR_H