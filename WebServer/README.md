# 服务器相关类 

## 相关流程封装-Util 

* 实现readn、writen函数
    * readn功能：因为一次read操作所返回的数据可能少于所要求的的数据，即使还没有达到文件尾端，所以就应该继续读取该文件，即readn
        * readn重载了c语言风格的char* buff和cpp风格的string buff
    * writen功能：一次write操作的返回值也可能少于指定输出的字节数，这并不是错
    误，所以需要继续写，即writen
        * writen同样重载了两种语言风格 

* 封装与sockfd相关的方法
    * 设置非阻塞
    * 设置Linger，即close或shutdown操作需要等到所有套接字里排队的消息成功发送或到达延迟时间才会返回
    * 关闭Nagle算法
        * Nagle算法：
            * 通过减少需要通过网络发送包的数量来提高TCP/IP传输的效率
            * 避免发送小的数据包，要求TCP连接上最多只能有一个未被确认的小分组，在该分组的确认到达之前不能发送其他的小分组，相反，TCP会收集小分组，并在确认到来时以一个分组的方式发出去
            * 适用于发送方发送大批量的小数据，并且接收方作出及时回应的场合
        * 当应用场景不是连续请求+应答的模型，而是需要实时的单项的发送数据并及时获取响应，就不适合Nagle算法，在本项目中，服务器用于发送文件，而不是大批量的小数据，所以需要禁用
    * 关闭连接写方向，此时连接处于“半关闭”状态
        * shutdown只会关闭连接，保留套接字和相关资源，但是会使套接字不可用

* 忽略掉SIGPIPE信号 
    * 产生SIGPIPE的原因：对一个对端已经关闭的socket调用两次write，第二次将会生成SIGPIPE，该信号默认结束进程，第一次write时会导致对端发送RST报文，表示对端已经调用close
    * 忽略SIGPIPE目的：避免进程推出 

* 设置服务器的IP和Port并与listen_fd绑定，且封装监听功能 

## Timer定时器 

* TimerNode定时器结点，一个定时器包括超时时间、删除标志位、请求事件 

* TimerManager定时器管理类，本质就是将定时器放在一个小根堆上，用于关闭超时请求 

## Channel事件分发器 

* channel是在epoll和TcpConnectin间起沟通作用，故也叫通道 

* 其他类通过调用channel的setCallback来建立和channel沟通关系 

* 每个Channel对象只属于一个EventLoop，因此每个Channel对象都只属于某一个IO线程 

## Epoll封装Epoll基本操作 

* 包括epoll实例的创建，事件在epoll注册表中的添加、删除、修改，返回事件活跃数 

* 分发处理函数
    * 该类中有存储Channel对象和持有该Channel对象的httpdata对象的两个数组
    * 调用分发处理函数时，首先遍历事件数组，获取当前事件的fd，再在Channel数组里寻找是否有该fd对应的Channel对象，如果有则将事件标记成就绪事件，并加入待返回的请求事件数组 

* 添加定时器
    * 将httpdata的持有者添加到小根堆中 

## EventLoop 

* 核心类 

* 采用eventfd的线程通信机制，支持read，write，及epoll相关等操作
    * 唤醒：通过在wakeupfd中写入unsigned long long的1转换成char*数组来唤醒 

* 在实例化对象时就要判断是否在该线程里已经存在一个EventLoop，如果存在就通知，如果不存在则继续实例化 

* 核心方法loop 
    * 首先通过断言判断是否当前EventLoop对象在当前的thread中
    * 获取到Epoll注册表中的活跃事件
    * 通过循环处理所有的活跃事件
    * 调用所有待处理事件的工作函数 

## EventLoopThread 

* 对EventLoop的再封装，可以创建一个IO线程，通过startLoop返回一个IO线程的loop，threadFunc开启loop循环

## EventLoopThreadPool 

* 事件循环线程池，管理所有客户端连接，每个线程都有唯一一个事件循环 

## HttpData 

* 利用有限状态机对http请求进行解析 
* 其中MimeType类是封装媒体类型，将文件后缀名对应的媒体类型做一个映射 
* 支持HTTP管线化
    * 管线化试讲多个HTTP请求整批送出的技术，而在传送过程中不需要先等待服务器回应
    * 只有GET和HEAD等要求可以进行管线化，这句话的本质是只有**幂等**的请求才能被管道化
    * 必须透过永久连线完成 

## Server 

* 开启服务器，即开启事件循环线程池 
* 该类的Channel负责接收连接 
* 处理新的连接，利用线程池中getNextLoop方法拿到下一个loop，用该loop来处理新的连接，将该loop给到HttpData对象里，然后将该对象设置成Channel持有者，最后将该loop放入待处理队列即可