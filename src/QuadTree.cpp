// QuadTree.cpp
// 四叉树空间索引模块 - 用于高效进行空间查询
//
// 功能:
// - 空间索引管理 - 使用四叉树组织组件的空间位置
// - 点查询 - 查找包含给定点的所有组件
// - 矩形查询 - 查找与给定矩形相交的所有组件
// - 动态更新 - 支持组件位置变更时的索引更新

#include "aether/QuadTree.h"
#include <algorithm>

namespace aether {

// 四叉树默认构造函数
QuadTree::QuadTree() {
    root_.bounds = Rect::infinite();  // 无限边界
    root_.isLeaf = true;
    nodeCount_ = 1;
}

// 四叉树带边界的构造函数
// 参数: bounds - 初始边界
QuadTree::QuadTree(const Rect& bounds) {
    root_.bounds = bounds;
    root_.isLeaf = true;
    nodeCount_ = 1;
}

// 检查两个矩形是否相交
// 参数: a, b - 要检查的两个矩形
// 返回值: 相交返回true，否则返回false
bool QuadTree::intersects(const Rect& a, const Rect& b) const {
    // 使用分离轴定理：如果两个矩形在任意轴上不重叠，则不相交
    return !(a.right() < b.left() || a.left() > b.right() ||
        a.bottom() < b.top() || a.top() > b.bottom());
}

// 获取项目所在的象限
// 参数:
//   nodeBounds - 节点边界
//   itemBounds - 项目边界
// 返回值: 象限索引(0-3)，跨象限返回-1
int QuadTree::getQuadrant(const Rect& nodeBounds, const Rect& itemBounds) const {
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
void QuadTree::subdivide(Node& node) {
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
    node.children[0]->bounds = Rect{ x, y, w, h };         // 左上
    node.children[1]->bounds = Rect{ x + w, y, w, h };     // 右上
    node.children[2]->bounds = Rect{ x, y + h, w, h };     // 左下
    node.children[3]->bounds = Rect{ x + w, y + h, w, h }; // 右下
    
    node.isLeaf = false;
    nodeCount_ += 4;
}

// 递归插入组件到四叉树
// 参数:
//   h - 组件句柄
//   bounds - 组件边界
//   node - 当前节点
//   depth - 当前深度
void QuadTree::insert(ComponentHandle h, const Rect& bounds, Node& node, int depth) {
    if (node.isLeaf) {
        // 如果是叶子节点，检查是否需要细分
        if (node.components.size() < static_cast<size_t>(kMaxComponentsPerLeaf) || depth >= kMaxDepth) {
            node.components.push_back(h);
            return;
        }
        
        // 细分当前节点
        subdivide(node);
        
        // 将旧组件重新插入到子节点中
        std::vector<ComponentHandle> oldComponents = std::move(node.components);
        node.components.clear();
        
        for (auto& oldHandle : oldComponents) {
            if (static_cast<size_t>(oldHandle.index) < allBounds_.size()) {
                insert(oldHandle, allBounds_[oldHandle.index], node, depth);
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
void QuadTree::remove(ComponentHandle h, Node& node) {
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
void QuadTree::queryNode(const Node& node, const Point& p, std::vector<ComponentHandle>& out) const {
    // 节点边界不包含查询点，直接返回
    if (!node.bounds.contains(p)) {
        return;
    }
    
    // 检查当前节点中的组件
    for (const auto& h : node.components) {
        if (static_cast<size_t>(h.index) < allBounds_.size()) {
            if (allBounds_[h.index].contains(p)) {
                out.push_back(h);
            }
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
void QuadTree::queryNode(const Node& node, const Rect& rect, std::vector<ComponentHandle>& out) const {
    // 节点边界不与查询矩形相交，直接返回
    if (!intersects(node.bounds, rect)) {
        return;
    }
    
    // 检查当前节点中的组件
    for (const auto& h : node.components) {
        if (static_cast<size_t>(h.index) < allBounds_.size()) {
            if (intersects(allBounds_[h.index], rect)) {
                out.push_back(h);
            }
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
void QuadTree::rebuild(const std::vector<ComponentHandle>& components,
    std::function<Rect(ComponentHandle)> getBounds) {
    clear();
    
    // 保存所有组件和边界
    allComponents_ = components;
    allBounds_.resize(components.size());
    
    // 收集所有组件的边界
    for (size_t i = 0; i < components.size(); ++i) {
        allBounds_[i] = getBounds(components[i]);
    }
    
    // 插入所有组件
    for (size_t i = 0; i < components.size(); ++i) {
        insert(components[i], allBounds_[i], root_, 0);
    }
}

// 更新组件的位置
// 参数:
//   h - 组件句柄
//   newBounds - 新的边界
void QuadTree::update(ComponentHandle h, const Rect& newBounds) {
    if (h.index >= 0 && h.index < static_cast<int32_t>(allBounds_.size())) {
        // 先移除再重新插入
        remove(h, root_);
        allBounds_[h.index] = newBounds;
        insert(h, newBounds, root_, 0);
    }
}

// 查询包含给定点的所有组件
// 参数: p - 查询点
// 返回值: 包含该点的组件列表
std::vector<ComponentHandle> QuadTree::query(const Point& p) const {
    std::vector<ComponentHandle> result;
    queryNode(root_, p, result);
    return result;
}

// 查询与给定矩形相交的所有组件
// 参数: rect - 查询矩形
// 返回值: 相交的组件列表
std::vector<ComponentHandle> QuadTree::query(const Rect& rect) const {
    std::vector<ComponentHandle> result;
    queryNode(root_, rect, result);
    return result;
}

// 清空四叉树
void QuadTree::clear() {
    root_ = Node();
    root_.bounds = Rect::infinite();
    root_.isLeaf = true;
    allComponents_.clear();
    allBounds_.clear();
    nodeCount_ = 1;
    maxDepth_ = 0;
}

} // namespace aether
