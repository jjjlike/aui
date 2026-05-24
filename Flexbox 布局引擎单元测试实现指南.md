以下是将 **Flexbox 布局引擎完整单元测试列表**（基础部分）与 **嵌套 Flexbox 详细单元测试用例**（高级部分）合并后的文档。合并时删除了重复内容（例如基础属性测试与嵌套测试中重复提及的简单示例），保留了各自独特的测试点，并按逻辑重新组织章节。

---

# Flexbox 布局引擎单元测试实现指南

> 本文档提供了实现 Flexbox 布局引擎所需的全部单元测试用例，覆盖基础特性、核心算法、边界情况以及复杂的嵌套场景。每个测试用例均包含：测试目标、输入条件、预期结果和断言方法。可直接用于编写 Google Test 测试代码。

---

## 一、测试环境与准备

- **测试框架**：Google Test（GTest）
- **辅助工具**：为每个测试创建一个 Flex 容器，添加若干子项，调用布局计算，然后断言子项的最终位置和尺寸（`left`, `top`, `width`, `height`）。
- **默认假设**：所有尺寸单位一致（如像素），不考虑浏览器默认样式。如无特殊说明，所有内外边距和边框均为 0。

---

## 二、容器属性测试（Flex Container）

### 2.1 `flex-direction`

| 测试用例 | 输入（flex-direction + 子项宽高） | 预期结果 |
|---------|--------------------------------|----------|
| `row` | 容器宽 600px，3 个子项宽各 100px，高各 50px | 子项水平排列，x 坐标依次为 0, 100, 200；y 坐标均为 0 |
| `row-reverse` | 同上 | 子项从右向左排列，x 坐标依次为 500, 400, 300 |
| `column` | 容器高 600px，3 个子项高各 100px，宽各 50px | 子项垂直排列，y 坐标依次为 0, 100, 200；x 坐标均为 0 |
| `column-reverse` | 同上 | 子项从下向上排列，y 坐标依次为 500, 400, 300 |

### 2.2 `flex-wrap`

| 测试用例 | 输入 | 预期结果 |
|---------|------|----------|
| `nowrap` | 容器宽 200px，3 个子项宽度各 100px，`flex-shrink: 0` | 溢出，子项超出容器边界 |
| `wrap` | 容器宽 250px，子项宽度各 100px，margin 0 | 第一行放 2 个子项（x=0,100），第二行放 1 个子项（x=0, y=子项高度） |
| `wrap-reverse` | 同上 | 第一行放 1 个子项（y=0），第二行放 2 个子项（y=子项高度，x=0,100） |
| 换行后对齐（多行） | 容器宽 400px，子项宽度 150px，共 3 个 | 第一行 2 个（x=0,150），第二行 1 个（x=0） |

### 2.3 `justify-content`

测试条件：容器宽 600px，3 个子项宽度固定 100px，无 margin，`flex-wrap: nowrap`。

| 值 | 预期子项 x 坐标序列（px） |
|----|--------------------------|
| `flex-start` | 0, 100, 200 |
| `flex-end` | 300, 400, 500 |
| `center` | 150, 250, 350 |
| `space-between` | 0, 250, 500 |
| `space-around` | 约 75, 275, 475（左右间距 75，中间 200） |
| `space-evenly` | 约 100, 300, 500（间距均为 100） |

### 2.4 `align-items`（单行）

容器高 200px，子项高度不同：item1高30，item2高50，item3高40。

| `align-items` 值 | 预期子项 y 坐标（px） |
|-----------------|----------------------|
| `stretch`（子项未设高度） | 子项高度拉伸为 200，y=0 |
| `flex-start` | 0, 0, 0 |
| `flex-end` | 170, 150, 160 |
| `center` | 85, 75, 80 |

> `baseline` 需按文字基线对齐，测试较复杂，暂不列入。

### 2.5 `align-content`（多行）

容器高 400px，`flex-wrap: wrap`，子项每行固定高度 50px，共两行。行间剩余空间 300px。

| `align-content` 值 | 预期行的 y 坐标（第一行起始，第二行起始） |
|--------------------|------------------------------------------|
| `stretch` | 行高拉伸为 200px，两行各占 200px，y=0 和 y=200 |
| `flex-start` | 0, 50 |
| `flex-end` | 300, 350 |
| `center` | 150, 200 |
| `space-between` | 0, 350 |
| `space-around` | 75, 275（间距 75, 150, 75） |
| `space-evenly` | 100, 300（间距 100, 100, 100） |

> **注意**：`align-content` 仅当存在多行且交叉轴有剩余空间时生效。单行时无效。

---

## 三、子项属性测试（Flex Items）

### 3.1 `order`

容器：`flex-direction: row`，三个子项宽度 100px，order 分别为 2, 0, 1。  
**预期**：显示顺序为索引1 (order0) → 索引2 (order1) → 索引0 (order2)，x坐标依次 0, 100, 200。

### 3.2 `flex-grow`

容器宽 600px，三个子项初始宽度 100px，剩余空间 300px。

| flex-grow 值 | 预期各子项最终宽度（px） |
|-------------|------------------------|
| 1,1,1 | 200, 200, 200 |
| 1,2,1 | 175, 250, 175 |
| 0,1,0 | 100, 400, 100 |
| 仅一个子项 grow=1 | 该子项占用全部剩余空间：100, 100+300, 100 |

### 3.3 `flex-shrink`

容器宽 600px，三个子项基础宽度（flex-basis）各 300px，总宽 900px，溢出 300px。

| flex-shrink 值 | 预期各子项最终宽度（px） |
|---------------|------------------------|
| 1,1,1 | 200, 200, 200 |
| 2,1,1 | 150, 225, 225 |
| 0,1,0 | 300, 150, 300 |

### 3.4 `flex-basis`

容器宽 600px，子项设 `flex-grow:1`。

| flex-basis 值 | 子项 width 设置 | 预期最终宽度 |
|--------------|----------------|--------------|
| `auto` | width=100px | 首先分配 100px，剩余 300px 按 grow 分配 |
| `auto` | 无 width | 按内容宽度（假设 0）计算 |
| 固定值 200px | width=100px（忽略） | 基准 200px，剩余 400px 分配 |
| `0%` | 任意 | 基准为 0，全部 600px 按 grow 分配 |

### 3.5 `align-self`

容器 `align-items: center`，容器高 200px。子项 A 高度 30，子项 B 设 `align-self: flex-start`，子项 C 设 `align-self: flex-end`。

**预期**：A.y=85, B.y=0, C.y=160。

---

## 四、布局计算核心算法测试

### 4.1 剩余空间分配（正剩余 + grow）

容器宽 500px，三个子项：
- Item1: `flex: 1` (basis 0, grow 1)
- Item2: `flex: 2` (basis 0, grow 2)
- Item3: `flex: 1` (basis 0, grow 1)  
**预期宽度**：125px, 250px, 125px（比例 1:2:1）。

### 4.2 负剩余空间收缩（溢出 + shrink）

容器宽 300px，三个子项各 `flex-basis: 200px`，shrink 比例 1,2,1。  
**预期宽度**：125px, 50px, 125px（计算：溢出300，分配比例 1:2:1 → 各减75,150,75）。

### 4.3 混合基础尺寸（部分 auto，部分固定）

容器宽 500px：
- ItemA: `width: 100px`, `flex-grow: 1`
- ItemB: `width: 100px`, `flex-grow: 2`
- ItemC: `width: 100px`, `flex-grow: 1`  
**预期**：剩余 200px 分配 1:2:1 → 最终 140, 180, 140。

### 4.4 最小尺寸约束（min-width / min-height）

容器宽 300px：
- ItemA: `flex-basis: 400px`, `min-width: 200px`, `flex-shrink: 1`
- ItemB: `flex-basis: 400px`, `min-width: 350px`, `flex-shrink: 1`  
**预期**：A 收缩到 200（被 min 卡住），B 收缩到 100？但因 B.min=350，实际 B 最终 350，总宽 550 > 300，溢出容器。测试用于验证 min 约束优先级。

### 4.5 内容保护（automatic minimum size）

容器宽 200px：
- Item: 内容宽度 150px（如文本），`flex-basis: 50px`, `flex-shrink: 1`  
**预期**：最终宽度 ≥ 150px（内容保护）。

### 4.6 宽高比保留（aspect-ratio）

容器 `flex-direction: row`，容器高 200px。
- Item: 图片无宽高，有宽高比 16/9，`flex-basis: auto`  
**预期**：基准宽度 = 200 * 16 / 9 ≈ 355.6px（若容器宽度足够）。

---

## 五、换行（Multi-line）复杂测试

### 5.1 换行后行的交叉轴对齐（align-content）
参见 2.5 节。

### 5.2 不同行内项目高度不一 + align-content: stretch

容器高 400px，wrap，第一行项目高度 30, 50；第二行项目高度 80, 40。`align-content: stretch`。  
**预期**：行高拉伸填满整个容器，第一行总高 = 第二行总高 = 200px。内部项目若无固定高度，则被拉伸至 200px。

### 5.3 换行时 flex-grow 在多行上的行为

容器宽 400px，子项宽度 150px，共 3 个，wrap。第一行两个，第二行一个。  
**预期**：第一行两个子项分配剩余 100px（各 200px），第二行子项宽度仍 150px（无剩余）。

---

## 六、边距（Margin）与边框（Border）测试

### 6.1 margin 对布局的影响

容器宽 600px，三个子项宽度各 100px，margin-right 分别为 20, 30, 0。  
**预期**：子项 x 坐标需考虑 margin 占据的空间。

### 6.2 margin: auto 的特殊行为

容器宽 600px，两个子项宽度 100px，第一项 `margin-right: auto`，第二项无 auto。  
**预期**：第一项右侧占据所有剩余空间，第二项紧贴右边缘。

### 6.3 负 margin

负 margin 可使项目溢出容器或重叠。测试其是否影响布局计算。

---

## 七、边缘与边界测试

| 测试场景 | 输入 | 期望行为 |
|---------|------|----------|
| 空容器 | 无子项 | 布局完成不崩溃，容器尺寸可计算 |
| 单个子项 | 容器宽 500，子项 flex-grow:1 | 子项宽度 500 |
| 零尺寸子项 | 子项 width:0，flex-grow:1 | 子项获得剩余空间 |
| 溢出交叉轴 | 容器高度固定 100，子项高度 200，wrap 无 | 溢出部分不可见（由渲染层处理） |
| 极大 shrink 值 | shrink = 100，basis 很大 | 收缩至 min-width 或内容边界 |

---

## 八、属性组合与简写测试

### 8.1 `flex` 简写值展开

测试每个简写值生成的 `grow/shrink/basis` 组合是否正确。例如：
- `flex: 1` → `1 1 0%`
- `flex: auto` → `1 1 auto`
- `flex: none` → `0 0 auto`
- `flex: 0 0 100px` → grow 0, shrink 0, basis 100px

### 8.2 `flex-flow` 简写

`flex-flow: column wrap` 应同时设置 `flex-direction: column` 和 `flex-wrap: wrap`。

---

## 九、嵌套 Flexbox 详细测试

以下测试用例覆盖内层容器作为外层子项时的各种交互行为，包括尺寸传递、对齐独立性、伸缩计算、最小尺寸约束、绝对定位、`margin: auto` 以及深层嵌套。

### 9.1 外层容器的布局影响内层容器的尺寸

#### 用例 1：外层 `flex-direction: row`，内层作为 flex 子项被分配宽度

**输入**：
- 外层容器：宽 500px，`flex-direction: row`
- 外层子项：A (宽 100px)，B (内层 Flex 容器，`flex-grow: 1`)
- 内层 B 包含三个子项：B1 (宽 50px)，B2 (`flex-grow: 1`)，B3 (宽 50px)

**预期**：
- B 宽度 = 400px
- B1 宽 50px，x=0；B2 宽 300px；B3 宽 50px，x=350（相对于 B）

#### 用例 2：外层 `flex-direction: column`，内层作为 flex 子项被分配高度

**输入**：
- 外层容器：高 400px，`flex-direction: column`
- 外层子项：A (高 80px)，B (内层 Flex 容器，`flex-grow: 1`)
- 内层 B 包含三个子项（垂直排列）：B1 (高 30px)，B2 (`flex-grow: 1`)，B3 (高 40px)

**预期**：
- B 高度 = 320px
- B1 高 30px，y=0；B2 高 250px；B3 高 40px，y=280

### 9.2 内层容器的对齐方式独立于外层

#### 用例 3：外层 `align-items: center`，内层容器内部的子项使用不同的 `align-self`

**输入**：
- 外层：宽 500px，高 300px，`align-items: center`
- 外层子项 B：内层 Flex 容器（`align-items: flex-start`），宽 400px，高 100px
- B 内部：B1 (高 30px)，B2 (`align-self: stretch`，高未设)

**预期**：
- B.y = 100（垂直居中）
- B1.y = 0 (相对于 B)，高 30
- B2 高拉伸至 100px，y=0

#### 用例 4：多层嵌套，外层 `justify-content: space-between` 不影响内层

**输入**：
- 外层：宽 600px，`justify-content: space-between`
- 外层子项：A (宽 100px)，B (内层 Flex 容器，宽 200px，`justify-content: center`)，C (宽 100px)
- B 内部：B1 (30px)，B2 (40px)，B3 (30px)

**预期**：
- A.x=0，B.x=200，C.x=500
- B 内部：B1.x=50，B2.x=80，B3.x=120（相对于 B）

### 9.3 内层容器的 `flex` 伸缩属性与外层交互

#### 用例 5：内层容器使用 `flex: 1` 参与外层剩余空间分配，同时内层自身布局使用 `flex` 简写

**输入**：
- 外层：宽 800px
- 外层子项：A (100px)，B (内层 Flex 容器，`flex: 1`)，C (100px)
- B 内部：B1 (`flex: 1`)，B2 (`flex: 2`)

**预期**：
- B 宽 = 600px
- B1 宽 = 200px，B2 宽 = 400px

#### 用例 6：内层容器固定宽度，但内部项目溢出时的收缩行为

**输入**：
- 外层：宽 300px
- 外层子项：A (100px)，B (内层 Flex 容器，`flex-basis: 250px`, `flex-shrink: 1`)
- B 内部：三个子项，各 `flex-basis: 100px`，`flex-shrink: 1`

**预期**：
- B 最终宽 = 200px（外层收缩后）
- B 内部三个子项各收缩至约 66.67px

### 9.4 嵌套中的最小尺寸约束传播

#### 用例 7：内层子项有内容保护导致外层无法收缩到过小尺寸

**输入**：
- 外层：宽 200px
- 外层子项 A：内层 Flex 容器，`flex-basis: 300px`, `flex-shrink: 1`
- 内层 A 包含子项 B：文本内容最小宽度 180px，`flex-basis: 50px`

**预期**：
- A 最终宽 = 200px（收缩 100px，未触及最小 180px）
- B 宽度 ≥ 180px

#### 用例 8：多层嵌套中，最内层的最小尺寸传递到外层导致溢出

**输入**：
- 根容器：宽 150px
- Level1：`flex-basis: 300px`, `flex-shrink: 1`
- Level2（Level1 内部）：`flex-basis: 250px`, `flex-shrink: 1`
- Leaf（Level2 内部）：文本最小宽度 200px，`flex-basis: 100px`, `flex-shrink: 1`

**预期**：
- Leaf 宽 = 200px → Level2 宽 = 200px → Level1 宽 = 200px → 超出根容器宽度（溢出）

### 9.5 嵌套中的绝对定位与相对定位

#### 用例 9：内层 Flex 容器作为相对定位容器，内部绝对定位元素相对于内层容器定位

**输入**：
- 外层：`display: flex`
- 内层 B：`position: relative`, 宽 300px，高 200px
- B 内部：正常子项 B1 (`flex: 1`)，绝对定位元素 Abs (`position: absolute`, `top:10`, `left:20`, 宽50,高30)

**预期**：
- B1 宽 = 300px（占满）
- Abs 不参与 Flex 布局，位置 (20,10) 相对于 B 左上角

### 9.6 嵌套中的边距合并与 `margin: auto`

#### 用例 10：内层容器使用 `margin: auto` 在外层中占据剩余空间

**输入**：
- 外层：宽 600px
- 外层子项：A (100px)，B (内层 Flex 容器，宽 200px，`margin-left: auto`)
- B 内部：两个子项各宽 80px

**预期**：
- B.x = 100 + 300 = 400px（左侧 margin 吸收剩余 300px）
- B 内部 B1.x=0, B2.x=80

#### 用例 11：多层嵌套中，内层内部子项使用 `margin: auto` 分布

**输入**：
- 外层：宽 800px，`justify-content: center`
- 外层子项 B：宽 400px
- B 内部：B1 (50px)，B2 (50px, `margin-left: auto`)，B3 (50px)

**预期**：
- B.x = 200
- B 内部：B1.x=0，B2.x=300（吸收剩余 250px），B3.x=350

### 9.7 复杂的深层嵌套综合测试

#### 用例 12：三层嵌套，混合方向、对齐、伸缩和换行

**输入**（详见原始描述）：
- 根容器 R：1000x600，`flex-direction: column`, `justify-content: space-between`
- R 子项：Header (h=80), Main (`flex-grow:1`, `flex-direction: row`, `justify-content: center`, `align-items: stretch`), Footer (h=60)
- Main 子项：Sidebar (w=200, `align-self: flex-start`, h=100), Content (内层 Flex 容器, `flex-grow:1`, `flex-direction: column`, `flex-wrap: wrap`, `align-content: flex-start`)
- Content 包含 6 个卡片（宽150,高100, margin=10）

**预期**（关键点）：
- Header.y=0, Footer.y=540, Main 高=460
- Sidebar 宽200, 高100, y=80（相对 Main 顶部）
- Content 宽=800, 高=460
- 第一行卡片 y=10, x 分别为 10,180,350,520；第二行 y=130

### 9.8 性能与边界测试（嵌套相关）

#### 用例 13：深度嵌套（10层）且每层都有多个子项

**输入**：动态生成 10 层嵌套，每层一个 Flex 容器，每层内有 3 个子项，其中一个是下一层容器。总节点数约 1024。赋予随机 `flex-direction` 和 `justify-content`。

**预期**：布局计算完成时间 < 100ms，无栈溢出，所有元素位置/尺寸为有效数值。

---

## 十、测试用例优先级建议

| 优先级 | 模块 | 数量（估算） |
|--------|------|-------------|
| P0（必须） | 基础方向、换行、主轴对齐、剩余空间分配（grow/shrink） | 约 30 |
| P1（重要） | 交叉轴对齐（align-items/align-self）、最小尺寸约束、margin | 约 20 |
| P2（常规） | align-content（多行）、order、flex-basis、简写属性、嵌套基础（用例1-4） | 约 20 |
| P3（高级） | 宽高比、负 margin、深度嵌套（用例5-13）、性能边界 | 约 15 |

---

**文档结束** – 以上用例可覆盖 Flexbox 布局引擎的所有关键特性和复杂场景，确保实现的正确性和鲁棒性。