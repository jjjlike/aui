#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include <memory>
#include <vector>
#include <functional>

namespace aether {

/**
 * 四叉树类
 * 
 * 用于空间分区，加速空间查询
 * 支持点查询和区域查询
 * 主要用于碰撞检测和命中测试
 */
class QuadTree {
public:
    /**
     * 四叉树节点结构
     * 
     * 包含边界、组件列表和子节点
     */
    struct Node {
        Rect bounds;                                  // 节点边界
        std::vector<ComponentHandle> components;     // 节点包含的组件
        std::unique_ptr<Node> children[4];          // 四个子节点（0-3）
        bool isLeaf = true;                          // 是否为叶子节点
    };
    
    /**
     * 默认构造函数
     */
    QuadTree();
    
    /**
     * 构造函数
     * @param bounds 四叉树边界
     */
    explicit QuadTree(const Rect& bounds);
    
    /**
     * 重建四叉树
     * @param components 要插入的组件列表
     * @param getBounds 获取组件边界的函数
     */
    void rebuild(const std::vector<ComponentHandle>& components,
                 std::function<Rect(ComponentHandle)> getBounds);
    
    /**
     * 更新组件位置
     * @param h 组件句柄
     * @param newBounds 新边界
     */
    void update(ComponentHandle h, const Rect& newBounds);
    
    /**
     * 查询点所在的所有组件
     * @param p 查询点
     * @return 包含该点的组件列表
     */
    std::vector<ComponentHandle> query(const Point& p) const;
    
    /**
     * 查询区域内的所有组件
     * @param rect 查询区域
     * @return 与区域相交的组件列表
     */
    std::vector<ComponentHandle> query(const Rect& rect) const;
    
    /**
     * 清空四叉树
     */
    void clear();
    
    /**
     * 获取节点数量
     * @return 总节点数
     */
    int getNodeCount() const { return nodeCount_; }
    
    /**
     * 获取最大深度
     * @return 四叉树最大深度
     */
    int getMaxDepth() const { return maxDepth_; }
    
private:
    static constexpr int kMaxDepth = 8;                  // 最大深度限制
    static constexpr int kMaxComponentsPerLeaf = 4;      // 叶子节点最大组件数
    
    /**
     * 插入组件到节点
     * @param h 组件句柄
     * @param bounds 组件边界
     * @param node 目标节点
     * @param depth 当前深度
     */
    void insert(ComponentHandle h, const Rect& bounds, Node& node, int depth);
    
    /**
     * 从节点中移除组件
     * @param h 组件句柄
     * @param node 目标节点
     */
    void remove(ComponentHandle h, Node& node);
    
    /**
     * 查询指定点的组件
     * @param node 开始查询的节点
     * @param p 查询点
     * @param out 输出结果列表
     */
    void queryNode(const Node& node, const Point& p, std::vector<ComponentHandle>& out) const;
    
    /**
     * 查询指定区域的组件
     * @param node 开始查询的节点
     * @param rect 查询区域
     * @param out 输出结果列表
     */
    void queryNode(const Node& node, const Rect& rect, std::vector<ComponentHandle>& out) const;
    
    /**
     * 分裂节点
     * @param node 要分裂的节点
     */
    void subdivide(Node& node);
    
    /**
     * 获取边界所在的象限
     * @param nodeBounds 节点边界
     * @param itemBounds 项目边界
     * @return 象限索引（0-3），如果跨多个象限返回-1
     */
    int getQuadrant(const Rect& nodeBounds, const Rect& itemBounds) const;
    
    /**
     * 检查两个矩形是否相交
     * @param a 矩形a
     * @param b 矩形b
     * @return 如果相交返回true
     */
    bool intersects(const Rect& a, const Rect& b) const;
    
    Node root_;                                // 根节点
    std::vector<ComponentHandle> allComponents_;  // 所有组件列表
    std::vector<Rect> allBounds_;              // 所有组件边界
    int nodeCount_ = 0;                        // 节点计数
    int maxDepth_ = 0;                         // 最大深度
};

}
