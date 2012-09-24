#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <QtCore>

using namespace std;

class Node {
public:
    Node(): x(0), y(0), z(0), r(1) {}
    Node(float x_, float y_, float z_, float r_): x(x_), y(y_), z(z_), r(r_) {}
    void print() const
    {
        printf("    <%f, %f, %f>, %f\n", x, y, z, r);
    }

public:
    float x;
    float y;
    float z;
    float r;
};

class Branch {
public:
    int branchLevel() const { return branch_level; }
    void setBranchLevel(int level) { branch_level = level; }
    bool empty() const { return nodeList.empty(); }
    void append(const Node& node) { nodeList.append(node); }
    void clear() { nodeList.clear(); }
    void print(const char* spline_method, const char* texture, const char* scale, float threshold=0.0) const;

private:
    QVector<Node> simplify(float threshold) const;
    void simplify(const QVector<Node>& nodes, const QVector<float>& thetas, int beg, int end, float threshold, QVector<Node>& res) const;
    void print(const QVector<Node>& nodes, const char* spline_method, const char* texture, const char* scale) const;
    
private:
    int branch_level;
    QVector<Node> nodeList;
};

float calculateTheta(const Node& first, const Node& second)
{
    float x = second.x - first.x;
    float y = second.y - first.y;
    float z = second.z - first.z;
    float res = acos(z / sqrt(x * x + y * y + z * z));
    return res;
}

void Branch::simplify(const QVector<Node>& nodes, const QVector<float>& thetas, int beg, int end, float threshold, QVector<Node>& res) const
{
    if (beg + 1 == end) {
        res.append(nodes[beg]);
        return;
    }
    if (*max_element(thetas.begin() + beg, thetas.begin() + end) - *min_element(thetas.begin() + beg, thetas.begin() + end) < threshold) {
        res.append(nodes[beg]);
        return;
    }
    int mid = (beg + end) / 2;
    simplify(nodes, thetas, beg, mid, threshold, res);
    simplify(nodes, thetas, mid, end, threshold, res);
}

QVector<Node> Branch::simplify(float threshold) const
{
    QVector<Node> nodes;
    QVector<float> thetas;
    for (int i = 0; i < nodeList.size() - 1; ++i)
        thetas.append(calculateTheta(nodeList[i], nodeList[i + 1]));
    simplify(nodeList, thetas, 0, thetas.size(), threshold, nodes);
    nodes.append(nodeList.last());
    return nodes;
}

void Branch::print(const char* spline_method, const char* texture, const char* scale, float threshold) const
{
    QVector<Node> simpNodeList = simplify(threshold);
    print(simpNodeList, spline_method, texture, scale);
}

void Branch::print(const QVector<Node>& nodes, const char* spline_method, const char* texture, const char* scale) const
{
    printf("sphere_sweep {\n");
    printf("    %s_spline\n", spline_method);
    printf("    %d,\n", nodes.size());
    foreach (const Node& node, nodes) 
        node.print();
    printf("    pigment { image_map {\"%s\"} scale %s }\n", texture, scale);
    printf("    finish {ambient 0.8 diffuse 0.8 phong 0.2}\n");
    printf("}\n\n");
}


int main(int argc, char* argv[])
{
    if (argc != 7) {
        printf("Usage: brc2pov.exe filename.brc cubic[linear] texture.jpg scale simp_threshold min_branch_level,max_branch_level");
        exit(-1);
    }

    QStringList branch_levels = QString(argv[6]).split(",");
    int min_branch_level = branch_levels.at(0).toInt();
    int max_branch_level = branch_levels.at(1).toInt();

    QFile brcfile(argv[1]);
    if (!brcfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printf("Can not open brc file %s", argv[1]);
        exit(-2);
    }

    Branch branch;
    QTextStream in(&brcfile);
    while (!in.atEnd()) {
        QStringList items = in.readLine().split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (items.empty())
            continue;
        if (items[0] == "g") {
            if (!branch.empty() && branch.branchLevel() >= min_branch_level && branch.branchLevel() <= max_branch_level) {
                branch.print(argv[2], argv[3], argv[4], QString(argv[5]).toFloat());
            }
            branch.clear();
        }
        if (items[0] == "p") {
            branch.setBranchLevel(items[5].toInt());
        }
        if (items[0] == "v") {
            branch.append(Node(items[1].toFloat(), items[2].toFloat(), items[3].toFloat(), items[4].toFloat()));
        }
    }

    if (branch.branchLevel() >= min_branch_level && branch.branchLevel() <= max_branch_level) {
        branch.print(argv[2], argv[3], argv[4], QString(argv[5]).toFloat());
    }

    brcfile.close();
    return 0;
}
