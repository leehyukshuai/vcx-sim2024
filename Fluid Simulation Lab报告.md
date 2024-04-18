# Fluid Simulation Lab报告

## Abstract

以非常正常的方式实现了任务要求：改变FlipRatio以控制模拟的具体实现、给粒子按照密度着色以及控制障碍物和粒子产生碰撞。要移动障碍物的话，需要通过按住Alt的同时移动鼠标左键。

另外稍微修改了下代码框架，使之更便于操作和编写程序，最后在300行以内完成了FluidSimulator的各个函数实现，感觉还行。

## Performance

关于模拟器的性能，在debug模式下，resolution为16时，帧率就会跌破25FPS，而当resolution进一步增加、帧率进一步下降时，模拟器会由于时间步长过大而产生崩坏（因为所有粒子都会堆积在底部，此时pushpart函数就无法再正常工作了）。为了解决这个问题，我在代码中设置了一段检查程序，如果帧率一直比较低，那么就限制dt为0.02s，并产生warning，这样就大大减轻了崩坏问题。

在Release模式下，Resolution可以跑到25左右，觉得很不错了已经。

另外我分析了下SimulateTimestep函数中各个子函数所占用的时间比值，发现pushParticlesApart函数基本上占用了70%以上的时间，其次是transferVelocities还有solveIncompressibility这两个函数。我本来想用并行化（thread库）来加快运算，将网格分成若干个小部分，每个线程单独处理一个部分，最后再合并结果，但未成行（太懒了x）。

## Reflection

刚发布的时候，硬着头皮上直接开始写，完成了第一版，很差劲，而且总是莫名崩溃，然后就弃坑了。前几天重新开始做，主要参考了Ten Minute Physics课程中html文档里的实现，实现的结果终于正常了，但我也没力气把它弄得更有趣一些了w。本来是想做个碗的形状作为容器来装下所有粒子，感觉是可以做到的，只用修改下最开始的setup和handleParticleCollisions就够了。但是这个lab真的很让人头大w，做完之后就再也不想碰了😥。

