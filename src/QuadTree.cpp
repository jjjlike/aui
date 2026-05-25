// JQuadTree.cpp
// 四叉树空间索引模块 - 用于高效进行空间查询
//
// 功能:
// - 空间索引管理 - 使用四叉树组织组件的空间位置
// - 点查询 - 查找包含给定点的所有组件
// - 矩形查询 - 查找与给定矩形相交的所有组件
// - 动态更新 - 支持组件位置变更时的索引更新

#include "aether/QuadTree.h"
#include <algorithm>

namespace jaether {

// 四叉树默认构造函数
JQuadTree::JQuadTree() {
    root_.bounds = JRect::infinite();  // 无限边界
    root_.isLeaf = true;
    nodeCount_ = 1;
}

// 四叉树带边界的构造函数
// 参数: bounds - 初始边界
JQuadTree::JQuadTree(const JRect& bounds) {
    root_.bounds = bounds;
    root_.isLeaf = true;
    nodeCount_ = 1;
}

// 检查两个矩形是否相交
// 参数: a, b - 要检查的两个矩形
// 返回值: 相交返回true，否则返回false
bool JQuadTree::intersects(const JRect& a, const JRect& b) const {
    // 使用分离轴定理：如果两个矩形在任意轴上不重叠，则不相交
    return !(a.right() < b.left() || a.left() > b.right() ||
        a.bottom() < b.top() || a.top() > b.bottom());
}

// 获取项目所在的象限
// 参数:
//   nodeBounds - 节点边界
//   itemBounds - 项目边界
// 返回值: 象限索引(0-3)，跨象限返回-1
int JQuadTree::getQuadrant(const JRect& nodeBounds, const JRect& itemBounds) const {
    float midX = nodeBounds.left() + nodeBounds.width / 2.0f;
    float midY = nodeBounds.top() + nodeBounds.height / 2.0f;
    
    // 检查是否完全在某个象限内
    bool left = itemBounds.right() <= midX;
    bool right = itemBounds.left() >= midX;
    bool top = itemBounds.bottom() <= midY;
    bool bottom = itemBounds.top() >= midY;
    
    if (top && left) return 0;    // 左上
    if (top && right) return 1;   // 右上
    if (bottom && left) return 2; // 左下
    if (bottom && right) return 3;// 右下
    
    return -1;  // 跨象限，需要保存在当前节点
}

// 细分一个节点，创建四个子节点
// 参数: node - 要细分的节点
void JQuadTree::subdivide(Node& node) {
    float x = node.bounds.left();
    float y = node.bounds.top();
    float w = node.bounds.width / 2.0f;
    float h = node.bounds.height / 2.0f;
    
    // 创建四个子节点
    for (int i = 0; i < 4; i++) {
        node.children[i] = std::make_unique<Node>();
        node.children[i]->isLeaf = true;
    }
    
    // 设置四个子节点的边界
    node.children[0]->bounds = JRect{ x, y, w, h };         // 左上
    node.children[1]->bounds = JRect{ x + w, y, w, h };     // 右上
    node.children[2]->bounds = JRect{ x, y + h, w, h };     // 左下
    node.children[3]->bounds = JRect{ x + w, y + h, w, h }; // 右下
    
    node.isLeaf = false;
    nodeCount_ += 4;
}

// 递归插入组件到四叉树
// 参数:
//   h - 组件句柄
//   bounds - 组件边界
//   node - 当前节点
//   depth - 当前深度
void JQuadTree::insert(JComponentHandle h, const JRect& bounds, Node& node, int depth) {
    if (node.isLeaf) {
        // 如果是叶子节点，检查是否需要细分
        if (node.components.size() < static_cast<size_t>(kMaxComponentsPerLeaf) || depth >= kMaxDepth) {
            node.components.push_back(h);
            return;
        }
        
        // 细分当前节点
        subdivide(node);
        
        // 将旧组件重新插入到子节点中
        std::vector<JComponentHandle> oldComponents = std::move(node.components);
        node.components.clear();
        
        for (auto& oldHandle : oldComponents) {
            auto key = std::make_pair(oldHandle.index, oldHandle.generation);
            auto it = componentBounds_.find(key);
            if (it != componentBounds_.end()) {
                insert(oldHandle, it->second, node, depth);
            }
        }
    }
    
    // 尝试插入到合适的子象限
    int quadrant = getQuadrant(node.bounds, bounds);
    if (quadrant >= 0 && !node.isLeaf && node.children[quadrant]) {
        insert(h, bounds, *node.children[quadrant], depth + 1);
    } else {
        // 跨象限或无子节点，保存在当前节点
        node.components.push_back(h);
    }
    
    // 更新最大深度
    if (depth > maxDepth_) {
        maxDepth_ = depth;
    }
}

// 递归从四叉树中移除组件
// 参数:
//   h - 要移除的组件句柄
//   node - 当前节点
void JQuadTree::remove(JComponentHandle h, Node& node) {
    // 从当前节点中移除
    auto it = std::find(node.components.begin(), node.components.end(), h);
    if (it != node.components.end()) {
        node.components.erase(it);
    }
    
    // 递归从子节点中移除
    if (!node.isLeaf) {
        for (auto& child : node.children) {
            if (child) {
                remove(h, *child);
            }
        }
    }
}

// 递归查询包含给定点的组件
// 参数:
//   node - 当前节点
//   p - 查询点
//   out - 输出结果列表
void JQuadTree::queryNode(const Node& node, const JPoint& p, std::vector<JComponentHandle>& out) const {
    // 节点边界不包含查询点，直接返回
    if (!node.bounds.contains(p)) {
        return;
    }
    
    // 检查当前节点中的组件
    for (const auto& h : node.components) {
        auto key = std::make_pair(h.index, h.generation);
        auto it = componentBounds_.find(key);
        if (it != componentBounds_.end() && it->second.contains(p)) {
            out.push_back(h);
        }
    }
    
    // 递归查询子节点
    if (!node.isLeaf) {
        for (const auto& child : node.children) {
            if (child) {
                queryNode(*child, p, out);
            }
        }
    }
}

// 递归查询与给定矩形相交的组件
// 参数:
//   node - 当前节点
//   rect - 查询矩形
//   out - 输出结果列表
void JQuadTree::queryNode(const Node& node, const JRect& rect, std::vector<JComponentHandle>& out) const {
    // 节点边界不与查询矩形相交，直接返回
    if (!intersects(node.bounds, rect)) {
        return;
    }
    
    // 检查当前节点中的组件
    for (const auto& h : node.components) {
        auto key = std::make_pair(h.index, h.generation);
        auto it = componentBounds_.find(key);
        if (it != componentBounds_.end() && intersects(it->second, rect)) {
            out.push_back(h);
        }
    }
    
    // 递归查询子节点
    if (!node.isLeaf) {
        for (const auto& child : node.children) {
            if (child) {
                queryNode(*child, rect, out);
            }
        }
    }
}

// 重建四叉树
// 参数:
//   components - 要索引的组件列表
//   getBounds - 获取组件边界的回调函数
void JQuadTree::rebuild(const std::vector<JComponentHandle>& components,
    std::function<JRect(JComponentHandle)> getBounds) {
    clear();
    
    // 保存所有组件和边界
    allComponents_ = components;
    
    // 收集所有组件的边界，使用句柄作为键
    for (const auto& h : components) {
        auto key = std::make_pair(h.index, h.generation);
        componentBounds_[key] = getBounds(h);
    }
    
    // 插入所有组件
    for (const auto& h : components) {
        auto key = std::make_pair(h.index, h.generation);
        insert(h, componentBounds_[key], root_, 0);
    }
}

// 更新组件的位置
// 参数:
//   h - 组件句柄
//   newBounds - 新的边界
void JQuadTree::update(JComponentHandle h, const JRect& newBounds) {
    auto key = std::make_pair(h.index, h.generation);
    auto it = componentBounds_.find(key);
    if (it != componentBounds_.end()) {
        // 先移除再重新插入
        remove(h, root_);
        componentBounds_[key] = newBounds;
        insert(h, newBounds, root_, 0);
    }
}

// 查询包含给定点的所有组件
// 参数: p - 查询点
// 返回值: 包含该点的组件列表
std::vector<JComponentHandle> JQuadTree::query(const JPoint& p) const {
    std::vector<JComponentHandle> result;
    queryNode(root_, p, result);
    return result;
}

// 查询与给定矩形相交的所有组件
// 参数: rect - 查询矩形
// 返回值: 相交的组件列表
std::vector<JComponentHandle> JQuadTree::query(const JRect& rect) const {
    std::vector<JComponentHandle> result;
    queryNode(root_, rect, result);
    return result;
}

// 清空四叉树
void JQuadTree::clear() {
    root_ = Node();
    root_.bounds = JRect::infinite();
    root_.isLeaf = true;
    allComponents_.clear();
    componentBounds_.clear();
    nodeCount_ = 1;
    maxDepth_ = 0;
}

} // namespace jaether
