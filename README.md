# -2023-

（不是官方判题器）为了更好的在本地测试代码，自己动手丰衣足食，写一个判题器和数据生成器。
可能会有bug。
- 修复了有些情况下识别不到有流未被发送的bug

注释出现乱码把编码格式改成ANSI。
- 编码已经改成 utf-8 了，不会出现乱码了

如果你搞不清楚目录，把程序放data文件夹里就对了。

直接运行程序运行完之后会瞬间退出，所以用命令行打开或者bat文件打开。
- 现在支持 clion 了，可以 ide 调试、也可以 cmake 自己编译

- Clion 运行的时候需要设置工作目录，否则会找不到 data 文件夹，工作路径为 $CMakeCurrentGenerationDir$
- 运行的时候需要手动新建 data 目录，否则会找不到 data 文件夹
