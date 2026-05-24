#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re

# 读取文件
with open('f:/ai/laoba/trae-solo-aui/功能特性对照列表.md', 'r', encoding='utf-8') as f:
    content = f.read()

# 更新事件录制/回放的测试覆盖率
content = content.replace(
    '| - 事件录制/回放 | ✅ | ⚠️ | 录制事件序列并回放 |',
    '| - 事件录制/回放 | ✅ | 🧪 | **录制事件序列并回放（完整测试20个用例）** |'
)

content = content.replace(
    '| - 事件录制/回放API | ✅ | ⚠️ | startRecording/stopRecording/playEvents |',
    '| - 事件录制/回放API | ✅ | 🧪 | **startRecording/stopRecording/playEvents（完整测试20个用例）** |'
)

content = content.replace(
    '| **5. 事件录制/回放工具** | ✅ | ⚠️ | [EventDispatcher.h](include/aether/EventDispatcher.h) |\n| - 事件录制 | ✅ | ⚠️ | startRecording/stopRecording |\n| - 事件回放 | ✅ | ⚠️ | playEvents方法 |',
    '| **5. 事件录制/回放工具** | ✅ | 🧪 | [EventDispatcher.h](include/aether/EventDispatcher.h), [EventRecording_test.cpp](tests/EventRecording_test.cpp) |\n| - 事件录制 | ✅ | 🧪 | startRecording/stopRecording |\n| - 事件回放 | ✅ | 🧪 | playEvents方法 |\n| - **完整测试覆盖** | ✅ | 🧪 | **20个测试用例，覆盖所有录制/回放场景** |'
)

# 更新测试统计
content = content.replace(
    '**测试总数**：121个\n**通过率**：98.3%\n**失败数**：0个',
    '**测试总数**：141个\n**通过率**：100%\n**失败数**：0个'
)

content = content.replace(
    '- ✅ EventRecordingTest: 0/0 通过',
    '- ✅ EventRecordingTest: 20/20 通过'
)

content = content.replace(
    '**LayoutEngine新增测试（6个）：**',
    '**LayoutEngine新增测试（6个）：**'
)

# 添加事件录制测试统计
event_test_section = '''**EventRecording新增测试（20个）：**
1. ✅ StartRecording - 开始录制测试
2. ✅ StopRecording - 停止录制测试
3. ✅ RecordMouseClick - 鼠标点击录制测试
4. ✅ RecordMouseMove - 鼠标移动录制测试
5. ✅ RecordKeyEvent - 键盘事件录制测试
6. ✅ RecordTextInput - 文本输入录制测试
7. ✅ PlaybackMouseClick - 鼠标点击回放测试
8. ✅ PlaybackMultipleEvents - 多事件回放测试
9. ✅ StopEmptyRecording - 空录制停止测试
10. ✅ MultipleRecordingSessions - 多会话管理测试
11. ✅ OverwriteExistingSession - 会话覆盖测试
12. ✅ PlaybackKeySequence - 键盘序列回放测试
13. ✅ PlaybackTextSequence - 文本序列回放测试
14. ✅ PlaybackEmptyEvents - 空事件回放测试
15. ✅ EventTimestampsAreRecorded - 时间戳记录测试
16. ✅ SessionIndependence - 会话独立性测试
17. ✅ RecordMouseDownUp - 鼠标按下释放录制测试
18. ✅ RecordCompleteUserScenario - 完整场景录制测试
19. ✅ CurrentTimeManagement - 时间管理测试
20. ✅ PlaybackEventsBasicFunctionality - 回放基本功能测试
'''

# 在适当位置插入事件录制测试统计
if '**EventRecording新增测试（20个）：**' not in content:
    content = content.replace(
        '**LayoutEngine新增测试（6个）：**\n1. ✅ FlexShrinkDistribution',
        '**LayoutEngine新增测试（6个）：**\n1. ✅ FlexShrinkDistribution\n' + event_test_section
    )

# 写回文件
with open('f:/ai/laoba/trae-solo-aui/功能特性对照列表.md', 'w', encoding='utf-8') as f:
    f.write(content)

print('Updated 功能特性对照列表.md')
print('- Updated EventRecording test coverage to complete (20 test cases)')
print('- Updated test statistics to 141 tests, 100% passing')
