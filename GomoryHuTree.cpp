#include <bits/stdc++.h>

using namespace std;

// Number of vertices
const int sz = 9;
// Adjacency matrix of the graph (specified as a constant for better visual perception).
// Nevertheless, a "vector" by "vector" is passed to the tree-building function for universality.
const int graph[sz][sz] =
    {
        {0,2,4,0,0,0,1,0,0},
        {2,0,6,11,0,0,0,0,0},
        {4,6,0,0,9,0,0,0,0},
        {0,11,0,0,7,0,2,0,0},
        {0,0,9,7,0,9,3,1,0},
        {0,0,0,0,9,0,0,8,0},
        {1,0,0,2,3,0,0,9,4},
        {0,0,0,0,1,8,9,0,3},
        {0,0,0,0,0,0,4,3,0}
    };

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Gomory-Hu
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

struct Edge;
struct Vertex;

typedef vector<int> Row;             // A row in the adjacency matrix
typedef vector<Row> Matrix;          // Adjacency matrix
typedef vector<Vertex *> VertexList; // List of vertices
typedef vector<Edge *> EdgeList;     // List of edges
vector<pair<string,string>> cutEdges;

// ----------------------------------------------------------------------------

// Vertex or a group of vertices

struct Vertex
{
    int id;           // Vertex identifier, used for displaying the algorithm's log and sorting vertices in the final tree
    VertexList group; // Vertices in the group (empty for a simple vertex)
    EdgeList edges;   // List of edges

    int flag;       // Auxiliary fields used in searches
    Vertex *parent; // Used in breadth-first search and elsewhere for simplifying operations

    // Constructors for a simple vertex and a group of vertices
    Vertex(int id) : id(id) {}
    Vertex(const VertexList &group) : id(-1), group(group) {}
};

// ----------------------------------------------------------------------------

// Edge (since edges are stored in vertex lists, only the second vertex is indicated;
// correspondingly, the other vertex points to a similar edge in the opposite direction)

struct Edge
{
    Vertex *vertex; // Adjacent vertex
    int c;          // Capacity
    int f;          // Flow

    Edge(Vertex *vertex, int c) : vertex(vertex), c(c), f(0) {}
};

// ----------------------------------------------------------------------------

// Result of finding the maximum flow

struct MinCut
{
    int f;        // Maximum flow between s and t
    Vertex *s;    // Pointer to s
    Vertex *t;    // Pointer to t
    VertexList A; // "Cut" half containing s
    VertexList B; // "Cut" half containing t

    MinCut(Vertex *s, Vertex *t) : s(s), t(t), f(0) {}
};


// ----------------------------------------------------------------------------

// Find a vertex in a list, returning an iterator (pointer)

VertexList::iterator findVertex(VertexList &set, Vertex *v)
{
    for (VertexList::iterator i = set.begin(); i != set.end(); ++i)
    {
        if (*i == v)
        {
            return i;
        }
    }
    return set.end();
}

// ----------------------------------------------------------------------------

// Set difference

VertexList setMinus(VertexList &set1, VertexList &set2)
{
    VertexList result;
    for (Vertex *v : set1)
    {
        // If a vertex from the left set is not found in the right set, add it to the result
        if (findVertex(set2, v) == set2.end())
        {
            result.push_back(v);
        }
    }
    return result;
}

// ----------------------------------------------------------------------------

// Set intersection

VertexList setMul(VertexList &set1, VertexList &set2)
{
    VertexList result;
    for (Vertex *v : set2)
    {
        // If a vertex from the right set is found in the left set, add it to the result
        if (findVertex(set1, v) != set1.end())
        {
            result.push_back(v);
        }
    }
    return result;
}

// ----------------------------------------------------------------------------

// Find an edge in a list, returning an iterator (pointer)

EdgeList::iterator findEdge(Vertex *v1, Vertex *v2)
{
    for (EdgeList::iterator i = v1->edges.begin(); i != v1->edges.end(); ++i)
    {
        if ((*i)->vertex == v2)
        {
            return i;
        }
    }
    return v1->edges.end();
}

// ----------------------------------------------------------------------------

// Create an edge V1 --[c]--> V2
// If addReverse == true, V2 --[c]--> V1 is also created
// If the edge already exists, c is added to its weight

void addEdge(Vertex *v1, Vertex *v2, const int c, bool addReverse = true)
{
    EdgeList::iterator i = findEdge(v1, v2);
    if (i != v1->edges.end())
    {
        (*i)->c += c;
    }
    else
    {
        v1->edges.push_back(new Edge(v2, c));
    }

    if (!addReverse)
    {
        return;
    }

    i = findEdge(v2, v1);
    if (i != v2->edges.end())
    {
        (*i)->c += c;
    }
    else
    {
        v2->edges.push_back(new Edge(v1, c));
    }
}

// ----------------------------------------------------------------------------

// Get the edge (if it exists) from V1 to V2

Edge *getEdge(Vertex *v1, Vertex *v2)
{
    EdgeList::iterator i = findEdge(v1, v2);
    return (i == v1->edges.end() ? nullptr : *i);
}

// ----------------------------------------------------------------------------

// Delete the edge from V1 to V2

void deleteEdge(Vertex *v1, Vertex *v2)
{
    EdgeList::iterator i = findEdge(v1, v2);
    if (i == v1->edges.end())
    {
        return;
    }

    delete (*i);
    v1->edges.erase(i);
}

// ----------------------------------------------------------------------------


// Delete a vertex from a set with the removal of edges

void deleteVertex(VertexList &set, Vertex *v)
{
    VertexList::iterator i = findVertex(set, v);
    if (i == set.end())
    {
        return;
    }

    // Iterate through edges connected to the vertex
    for (Edge *e : v->edges)
    {
        // Delete the reverse edge, then delete the direct edge
        deleteEdge(e->vertex, v);
        delete e;
    }

    // Delete the vertex
    delete (*i);
    set.erase(i);
}

// ----------------------------------------------------------------------------

// Delete the entire graph (all vertices and edges in the set)

void deleteVertexList(VertexList &set)
{
    for (Vertex *v : set)
    {
        for (Edge *e : v->edges)
        {
            delete e;
        }
        delete v;
    }
    set.clear();
}

// ----------------------------------------------------------------------------

// Unpack groups in a set {{1, 2}, {3}} --> {1, 2, 3}

VertexList extractGroups(const VertexList &set)
{
    VertexList result;

    for (Vertex *v : set)
    {
        if (v->group.size() > 0)
        {
            for (Vertex *subv : v->group)
            {
                result.push_back(subv);
            }
        }
        else
        {
            result.push_back(v);
        }
    }

    return result;
}

// ----------------------------------------------------------------------------

// Create an adjacency matrix from a set of vertices

Matrix vertexListToMatrix(const VertexList &set)
{
    int n = static_cast<int>(set.size());
    Matrix m(n, Row(n, 0));

    // Number vertices in the flag field, then fill the matrix
    for (int i = 0; i < n; i++)
    {
        set[i]->flag = i;
    }

    for (Vertex *v : set)
    {
        for (Edge *e : v->edges)
        {
            m[v->flag][e->vertex->flag] = e->c;
        }
    }

    return m;
}

// ----------------------------------------------------------------------------

// Sort the list by vertex id

void sortListById(VertexList &set)
{
    size_t n = set.size();

    for (size_t i = 0; i < n - 1; i++)
    {
        size_t k = i;
        for (size_t j = i + 1; j < n; j++)
        {
            if (set[j]->id < set[k]->id)
            {
                k = j;
            }
        }
        if (k != i)
        {
            Vertex *v = set[k];
            set[k] = set[i];
            set[i] = v;
        }
    }
}

// ----------------------------------------------------------------------------
// String representation of a vertex for reporting

string vertexToStr(const Vertex *vertex)
{
    stringstream s;
    s << "{";
    if (vertex->group.size() == 0)
    {
        s << vertex->id << "}";
        return s.str();
    }
    else
    {
        for (Vertex *v : vertex->group)
        {
            s << vertexToStr(v) << ",";
        }
        return s.str().substr(0, s.str().size() - 1) + "}";
    }
}

// ----------------------------------------------------------------------------

// String representation of a set of vertices for reporting

string vertexListToStr(const VertexList &set)
{
    stringstream s;
    s << "[ ";
    for (Vertex *v : set)
    {
        s << vertexToStr(v) << ", ";
    }
    return s.str().substr(0, s.str().size() - 2) + " ]";
}

// Function to convert a matrix to a string representation
string matrixToStr(const Matrix &m)
{
    int n = static_cast<int>(m.size());
    stringstream s;
    for (int i = 0; i < n; i++)
    {
        s << "    ";
        for (int j = 0; j < n; j++)
            s << setw(3) << m[i][j];
        s << endl;
    }
    return s.str();
}

// Function to find the shortest path from s to t
// It returns a queue formed during the search
VertexList findPath(const VertexList &set, Vertex *s, Vertex *t)
{
    // Clear previous search markings
    for (Vertex *v : set)
    {
        v->parent = nullptr;
        v->flag = 0;
    }

    size_t i = 0;
    VertexList queue;

    // Add the starting vertex to the queue and mark it
    s->flag = 1;
    queue.push_back(s);

    // Process the elements in the queue
    while (i < queue.size())
    {
        // Add all adjacent vertices with available capacity to the queue
        for (Edge *e : queue[i]->edges)
        {
            Vertex *v = e->vertex;
            if (v->flag == 0 && e->c > 0)
            {
                v->parent = queue[i];
                v->flag = 1;
                queue.push_back(v);

                // If the adjacent vertex is t, a shortest path is found
                if (v == t)
                    return queue;
            }
        }
        // Move to the next vertex in the queue
        i++;
    }

    return queue;
}

// Function to find the maximum flow and its min cut between vertices 0 and 1 in the set
MinCut findMinCut(VertexList &set)
{
    MinCut result(set[0], set[1]);

    for (;;)
    {
        // Find the shortest path between s and t
        VertexList path = findPath(set, result.s, result.t);
        if (result.t->parent == nullptr)
            break;

        // Calculate the minimum capacity along the path
        int min_c = INT_MAX;
        Vertex *v = result.t;
        EdgeList edges;

        

        while (v->parent != nullptr)
        {
            edges.push_back(getEdge(v, v->parent));
            Edge *e = getEdge(v->parent, v);
            edges.push_back(e);
            if (e->c < min_c)
                min_c = e->c;
            v = v->parent;
        }

        
        v = result.t;
        while (v->parent != nullptr)
        {
            Edge *e = getEdge(v->parent, v);
            if (e->c == min_c){
                cutEdges.push_back({vertexToStr(v->parent),vertexToStr(v)});
            }
            v = v->parent;
        }


        // Update the flow and capacities
        result.f += min_c;
        for (Edge *e : edges)
        {
            e->f += min_c;
            e->c -= min_c;
        }
    }

    // Find sets A and B from the min cut
    result.B = findPath(set, result.t, result.s);
    result.A = setMinus(set, result.B);

    // Restore capacities after the search
    for (Vertex *v : set)
    {
        for (Edge *e : v->edges)
        {
            e->c += e->f;
            e->f = 0;
        }
    }

    return result;
}
// Build the Gomory-Hu tree
Matrix buildGomoryHuTree(const Matrix &g)
{
    VertexList Vg = VertexList();
    VertexList Vt = VertexList();

    size_t n = g.size();
    for (size_t i = 0; i < n; i++)
    {
        Vg.push_back(new Vertex(i));
        for (size_t j = 0; j < i; j++)
        {
            if (g[i][j] != 0)
                addEdge(Vg[i], Vg[j], g[i][j]);
        }
    }

    Vt.push_back(new Vertex(Vg));

    cout << endl
         << "Step 1: Vt= " << vertexListToStr(Vt) << endl
         << endl;

    for (;;)
    {
        cout << "---------------------------------------------------------------" << endl
             << endl;

        // Step 2: Select a group of vertices from Vt where more than 1 vertex exists
        VertexList::iterator i = Vt.begin();
        VertexList::iterator j = Vt.end();
        while (i != j && (*i)->group.size() < 2)
            i++;
        if (i == j)
            break;

        Vertex *x = *i;

        cout << "Step 2: X = " << vertexListToStr(x->group) << endl
             << endl;

        // Step 3: Build the graph G
        VertexList G = VertexList();

        for (Vertex *v : x->group)
        {
            v->parent = new Vertex(VertexList(1, v));
            G.push_back(v->parent);
        }

        for (Edge *e : x->edges)
        {
            deleteEdge(e->vertex, x);
            Vertex *z = new Vertex(extractGroups(findPath(Vt, e->vertex, x)));
            addEdge(e->vertex, x, e->c, false);

            for (Vertex *v : z->group)
                v->parent = z;
            G.push_back(z);
        }

        for (Vertex *z : G)
        {
            for (Vertex *v : z->group)
            {
                for (Edge *e : v->edges)
                {
                    if (z != e->vertex->parent)
                        addEdge(z, e->vertex->parent, e->c, false);
                }
            }
        }

        cout << "Step 3: G = " << vertexListToStr(G) << endl
             << endl;
        cout << matrixToStr(vertexListToMatrix(G)) << endl
             << endl;

        // Step 4: Find the min cut with max flow in G
        MinCut cut = findMinCut(G);
        cut.A = extractGroups(cut.A);
        cut.B = extractGroups(cut.B);

        cout << "Step 4: s-t   = " << vertexToStr(cut.s) << "-" << vertexToStr(cut.t) << endl;
        cout << "        max_f = " << cut.f << endl;
        cout << "        A     = " << vertexListToStr(cut.A) << endl;
        cout << "        B     = " << vertexListToStr(cut.B) << endl
             << endl;

        deleteVertexList(G);

        // Step 5: Update the tree

        Vertex *XA = new Vertex(setMul(x->group, cut.A));
        Vertex *XB = new Vertex(setMul(x->group, cut.B));
        addEdge(XA, XB, cut.f);

        for (Edge *e : x->edges)
        {
            Vertex *v = e->vertex;
            addEdge((findVertex(cut.A, v->group[0]) != cut.A.end() ? XA : XB), v, e->c);
        }

        Vt.insert(Vt.insert(i, XB), XA);
        deleteVertex(Vt, x);

        cout << "Step 5: Vt= " << vertexListToStr(Vt) << endl
             << endl;
        cout << matrixToStr(vertexListToMatrix(Vt)) << endl
             << endl;
    }

    for (Vertex *v : Vt)
    {
        v->id = v->group[0]->id;
        v->group.clear();
    }

    sortListById(Vt);
    Matrix m = vertexListToMatrix(Vt);

    cout << "Result:" << endl
         << endl;
    cout << "Vt= " << vertexListToStr(Vt) << endl
         << endl;
    cout << matrixToStr(m) << endl
         << endl;

    deleteVertexList(Vg);
    deleteVertexList(Vt);

    return m;
}



void findMinKCut(Matrix M, int k){

    if(k == 1){
        cout << 0 << endl;
        return;
    }
    else if(k > M.size()){
        cout << "k can not be greater than no. of nodes in a graph" << endl;
        return;
    }

    vector<int> st;
    set<pair<int,int>> x;
    for(int i=0;i<M.size();i++){
        for(int j=0;j<M[i].size();j++){
            if(M[i][j] != 0){
                if(x.find({i,j}) == x.end() && x.find({j,i}) == x.end()){
                    st.push_back(M[i][j]);
                    x.insert({i,j});
                }
            }
        }
    }

    sort(st.begin(),st.end());
    
    int sum = 0;
    int count = 0;
    for(auto &x : st){
        sum += x;
        count++;
        if(count == k - 1) break;
    }

    cout << "Min K cut for k = " << k << " is " << sum << endl;
}

bool dfs(int node, int tar, vector<int> &vis, Matrix &m, int &ans){
    vis[node] = 1;
    if(node == tar) return true;

    for(int j=0;j<m[node].size();j++){
        if(m[node][j] != 0){
            if(!vis[j] && dfs(j,tar,vis,m,ans)){
                ans = min(ans,m[node][j]);
                return true;
            }
        }
    }

    return false;
}


void findMinCut_(Matrix m, int a, int b){
    if(a < 0 || b < 0 || a >= m.size() || b >= m.size() || a == b){
        printf("Enter valid nodes\n");
        return;
    }

    int n = m.size();
    int ans = INT_MAX;
    vector<int> vis(n,0);
    dfs(a,b,vis,m,ans);

    cout << "Min cut between " << a << ' '  << "and " << b << ": " << ans << endl;

    return;
}




int main(int argc, char *argv[])
{
    // Check if the graph is undirected
    for (int j = 0; j < sz; j++)
    {
        if (graph[j][j] != 0)
        {
            cout << "Error: Loop in vertex " << j << endl;
            return 0;
        }

        for (int i = 0; i < j; i++)
        {
            if (graph[i][j] != graph[j][i])
            {
                cout << "Error: Different weights in edge " << i << "-" << j << endl;
                return 0;
            }

            if (graph[i][j] < 0)
            {
                cout << "Error: Negative weight in edge " << i << "-" << j << endl;
                return 0;
            }
        }
    }

    Matrix g(sz, Row(sz));
    for (int i = 0; i < sz; i++)
    {
        for (int j = 0; j < sz; j++)
        {
            g[i][j] = graph[i][j];
        }
    }

    Matrix m = buildGomoryHuTree(g);
    findMinKCut(m,3);
    findMinCut_(m,0,2);

    // cout << cutEdges.size() << endl;
    // for(int i=0;i<cutEdges.size();i++){
        
    //     cout << "[ " <<  cutEdges[i].first << " , " << cutEdges[i].second << " ]" << endl;
    // }

    return 0;
}