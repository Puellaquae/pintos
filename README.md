# pintos

Pintos 2021/2022(1) 操作系统 大实验

## 说明 
* 此项目使用了 [pintos-anon](pintos-os.org) 的版本。并为 userprog 移植了 CS162 Fall 2021（如没有额外说明则 CS162 指 Fall 2021 学期的 CS162）额外的 do-nothing，iloveos，fp-\*，stack-align-\* 等 16 个测试点，但未为 stack-align-* 系列测试使用 CS162 指定的 clang 编译器及编译参数。
* 另外部分代码仅为针对学校 autograder 的特殊处理而非系统实现所需。
* 由于 CS162 的 pintos 与 pintos-anon 以及网上通常看到的相差过大（特别是 threads 部分），自 commit [0755189](https://github.com/Puellaquae/pintos/commit/07551897eef0900f4f79895907c782aca7631b11) 起，threads、userprog 和 filesys 将会采用 CS162 的要求内容，vm 使用 CS140 的内容（因为 CS162 没有这部分）。
* commit [2c2a83](https://github.com/Puellaquae/pintos/commit/2c2a83810bd00e260e26c1094bc77d3e5cab59d6) 及之前的内容为 CS140 下 threads 和 userprog 的内容。
* 如果你想要参考我的做法但又不想看到具体实现，可以看我[博客](https://github.com/Puellaquae/puellaquae.github.io)中的实验记录。

## DONE
* CS140 要求下的 threads
* userprog。

## TODO
* CS162 threads 的额外部分
* filesys
* vm
