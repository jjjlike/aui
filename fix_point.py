#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re

# 读取文件
with open('f:/ai/laoba/trae-solo-aui/tests/EventRecording_test.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# 修复Point比较
fixed_lines = []
for i, line in enumerate(lines):
    # 修复: EXPECT_EQ(events[0].mouseEvent.position, Point{100, 100});
    match = re.search(r'EXPECT_EQ\(events\[(\d+)\]\.mouseEvent\.position, Point\{(\d+), (\d+)\}\);', line)
    if match:
        idx, x, y = match.groups()
        indent = ' ' * (len(line) - len(line.lstrip()))
        fixed_lines.append(f'{indent}EXPECT_EQ(events[{idx}].mouseEvent.position.x, {x});\n')
        fixed_lines.append(f'{indent}EXPECT_EQ(events[{idx}].mouseEvent.position.y, {y});\n')
        continue
    
    # 修复: EXPECT_EQ(sessions["independent_a"][0].mouseEvent.position, Point{100, 100});
    match = re.search(r'EXPECT_EQ\(sessions\["([^"]+)"\]\[(\d+)\]\.mouseEvent\.position, Point\{(\d+), (\d+)\}\);', line)
    if match:
        session, idx, x, y = match.groups()
        indent = ' ' * (len(line) - len(line.lstrip()))
        fixed_lines.append(f'{indent}EXPECT_EQ(sessions["{session}"][{idx}].mouseEvent.position.x, {x});\n')
        fixed_lines.append(f'{indent}EXPECT_EQ(sessions["{session}"][{idx}].mouseEvent.position.y, {y});\n')
        continue
    
    fixed_lines.append(line)

# 写回文件
with open('f:/ai/laoba/trae-solo-aui/tests/EventRecording_test.cpp', 'w', encoding='utf-8') as f:
    f.writelines(fixed_lines)

print('Fixed Point comparisons')
