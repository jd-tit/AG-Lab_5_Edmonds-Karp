#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <optional>
#include <algorithm>

constexpr int INFTY = INT_MAX / 2;

size_t get_key(int x, int y){
//    return std::to_string(x) + "," + std::to_string(y);
return x*10000+y;
}

void from_key(size_t key, int& x, int& y){
    x = key / 10000;
    y = key % 10000;
}

class Node{
public:
    int label;
    Node* parent;
    bool visited;
    int distance;
    Node(int label): label(label){
    }
};

class Graph {
public:
    std::vector<Node> nodes;
    std::vector<std::vector<int>> lists;
    int size;

    explicit Graph(int size): size(size), lists(std::vector<std::vector<int>>(size)){
        for(int i = 0; i < size; ++i){
            nodes.emplace_back(i);
        }
    }

    void add_edge(int x, int y){
        lists[x].emplace_back(y);
    }

    std::vector<int>& get_list_of(int x){
        return lists[x];
    }

    bool has_edge(int x, int y){
        return std::find(lists[x].begin(), lists[x].end(), y) != lists[x].end();
    }

    static Graph residual_net(Graph& g, std::unordered_map<size_t, int>& caps, std::unordered_map<size_t, int>& flow, std::unordered_map<size_t, int>& ff){
        Graph gf(g.size);
//        ff.clear();
//        delete *ff;
//        *ff = new int[g.size * g.size];
    if(ff.empty()) {
        for (const auto &e: flow) {
            int x, y;
            auto key = e.first;
            from_key(e.first, x, y);

            int flow_x_y = e.second;

            if (flow_x_y < caps.at(key)) { //edge is not saturated, then add edge in same direction
                gf.add_edge(x, y);
                ff.emplace(std::make_pair(key, caps.find(key)->second - flow_x_y));
//                ff[key] = caps[key] - flow_x_y;

            }
            if (flow_x_y > 0) { // there is SOME flow from x to y, so add edge YX
                gf.add_edge(y, x);
                ff.emplace(std::make_pair(get_key(y, x), caps.find(key)->second - flow_x_y));
//                ff[get_key(y, x)] = caps[key] - flow_x_y;

            }
        }
    } else { //Super mega edit
        for (const auto &e: flow) {
            auto key_x_y = e.first;
            int x, y;
            from_key(key_x_y, x, y);
            auto key_y_x = get_key(y, x);

            if(ff.contains(key_x_y)){
                auto diff = caps.at(key_x_y) - flow.at(key_x_y);
                if(diff > 0) {
                    ff.at(key_x_y) = diff;
                    gf.add_edge(x, y);
                }
                if(diff < caps.at(key_x_y)) {
                    ff[key_y_x] = flow.at(key_x_y);
                    gf.add_edge(y, x);
                }
            }
        }
    }
        return gf;
    }
};


std::optional<std::vector<int>> BFS(Graph& g, Node& start, Node& stop){
    for(auto& n : g.nodes){
        n.visited = false;
        n.parent = nullptr;
        n.distance = INFTY;
    }

    start.visited = true;
    start.distance = 0;
    std::queue<int> queue;

    queue.push(start.label);

    while(!queue.empty()){
        int i = queue.front();
        queue.pop();
        for(auto n_index : g.lists[i]){
            auto& n = g.nodes[n_index];

            if(!n.visited){
                n.visited = true;
                n.parent = &g.nodes[i];
                n.distance = g.nodes[i].distance + 1;
                queue.push(n_index);
            }
        }
        g.nodes[i].visited = true;
    }

    std::vector<int> result;
    Node* crt = &stop;
    while(crt->parent != nullptr){
        result.push_back(crt->label);
        crt = crt->parent;
    }
    result.push_back(crt->label);
    if(result.size() > 1){
        std::reverse(result.begin(), result.end());
        return result;
    }
    return std::nullopt;
}



int get_min_capacity(Node& final, std::unordered_map<size_t, int>& caps){
    Node* crt = &final;
    int min = INFTY;
    while(crt->parent != nullptr){
        auto crt_cap = caps[get_key(crt->parent->label, crt->label)];
        min = std::min(min, crt_cap);
        crt = crt->parent;
    }
    return min;
}

int EK(Graph& g, std::unordered_map<size_t, int>& flow, std::unordered_map<size_t, int>& caps){
//    source is 0, sink is V-1
    int i = 0;
    for(auto& list : g.lists){
        for(auto& e : list){
            auto key = get_key(i, e);//ring(i) + "," + std::to_string(e);
            flow[key] = 0;
        }
        ++i;
    }

    int max_flow = 0;
    std::unordered_map<size_t, int> ff;
    auto gf = Graph::residual_net(g, caps, flow, ff);

    auto path = BFS(gf, gf.nodes[0], gf.nodes.back());

    while(path.has_value()){
        int min_cap = INT_MAX;
        for(i = 0; i < path->size() -1; ++i){ //find path min capacity
            int x = path->at(i), y = path->at(i+1);
            auto key = get_key(x, y);
            min_cap = std::min(min_cap, ff[key]);
        }

        max_flow += min_cap;

        //MODIFY FLOWS
        for(i = 0; i < path->size() -1; ++i){
            int x = path->at(i), y = path->at(i+1);
            auto key = get_key(x, y);
            if(g.has_edge(x, y))
                flow.at(key) += min_cap;
            else if(g.has_edge(y, x)){
                flow.at(get_key(y, x)) -= min_cap;
            }
        }

        gf = Graph::residual_net(g, caps, flow, ff);
        path = BFS(gf, gf.nodes[0], gf.nodes.back());
    }

//    for(auto e : g.lists[0]){
//        auto key = get_key(0, e);
//        max_flow += flow[key];
//    }
    return max_flow;
}


int main(int argc, char** argv) {
    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);

    if(in.is_open() && out.is_open()){
        int node_count, edge_count;
        in >> node_count >> edge_count;
        Graph graph(node_count);

        std::unordered_map<size_t, int> caps;
        std::unordered_map<size_t, int> flow;

        for(int i = 0; i < edge_count; ++i){
            int x, y, c;
            in >> x >> y >> c;
            graph.add_edge(x, y);

            caps.emplace(std::make_pair(get_key(x, y), c));
//            caps[get_key(x, y)] = c;
        }

        int max_flow = EK(graph, flow, caps);

        std::cout << "Max flow is: " << max_flow;


        return 0;
    }

    return 1;
}
