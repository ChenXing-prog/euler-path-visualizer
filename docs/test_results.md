# 一笔画问题算法测试结果

## 第一部分：固定测试用例结果表

| 编号 | 测试类型 | 输入规模 | 预期结果 | 实际结果 | 是否通过 |
|---|---|---|---|---|---|
| 用例1 | 欧拉回路，三角形图 | P=3, Q=3 | Yes，欧拉回路，edgePath 长度为 3 | hasEulerPath=true; isCircuit=true; edgePath长度=3; 顶点路径=1->2->3->1; 边序列=e1->e2->e3 | 通过 |
| 用例2 | 欧拉通路，两个奇度顶点 | P=4, Q=5 | Yes，欧拉通路，edgePath 长度为 5 | hasEulerPath=true; isCircuit=false; edgePath长度=5; 顶点路径=1->2->3->1->4->3; 边序列=e1->e2->e3->e4->e5 | 通过 |
| 用例3 | 不可一笔画，奇度顶点数量为 4 | P=4, Q=3 | No | hasEulerPath=false; isCircuit=false; edgePath长度=0 | 通过 |
| 用例4 | 不可一笔画，非孤立顶点不连通 | P=4, Q=2 | No | hasEulerPath=false; isCircuit=false; edgePath长度=0 | 通过 |
| 用例5 | 含重边的欧拉回路 | P=2, Q=2 | Yes，欧拉回路，edgePath 长度为 2 | hasEulerPath=true; isCircuit=true; edgePath长度=2; 顶点路径=1->2->1; 边序列=e1->e2 | 通过 |
| 用例6 | 空边图 | P=5, Q=0 | Yes，欧拉回路，edgePath 长度为 0 | hasEulerPath=true; isCircuit=true; edgePath长度=0; 顶点路径=1; 边序列=空 | 通过 |
| 用例7 | 单边图 | P=2, Q=1 | Yes，欧拉通路，edgePath 长度为 1 | hasEulerPath=true; isCircuit=false; edgePath长度=1; 顶点路径=1->2; 边序列=e1 | 通过 |
| 用例8 | 较复杂欧拉回路 | P=5, Q=6 | Yes，欧拉回路，edgePath 长度为 6 | hasEulerPath=true; isCircuit=true; edgePath长度=6; 顶点路径=1->2->3->4->5->3->1; 边序列=e1->e2->e4->e5->e6->e3 | 通过 |

## 第二部分：随机测试结果汇总

随机测试组数：1000  
通过组数：1000  
失败组数：0  
随机种子：20260518  
结论：随机生成的图中，算法判定结果均与欧拉通路定理一致；对于判定为可一笔画的图，输出路径均通过合法性检查。
