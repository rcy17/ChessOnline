# Chess Online

本内容为2019夏《程序设计训练》第二周的大作业，作业要求见《网络国际象棋对战v2.0.pptx》，主要是完成一个简单的双人国际象棋。

我确保了我的程序支持真实的网络对战，即在服务器端有公网IP的情况下能够远程对战，助教实际验收时是在同一台电脑上跑两个程序，监听127.0.0.1进行的。

我的程序完成了所有基础项与加分项，这里吐槽一下基础项里有对IP的鲁棒性要求，但其实没有测试。我在实现这个是用到了个很有趣的trick，相比很多同学用正则表达式来完成，我使用了如下几行代码：

```C++
void ConnectWindow::connectHost()
{
    auto text = ui->lineEdit->text();
    address.setAddress(text);

    if(address.toString()!=text)
    {
        QMessageBox::warning(this, "wrong address", "please input legal ip address!");
        return;
    }
    // other code
}
```

核心代码在于`address.toString()!=text`，这是使用Qt自带的地址转换来判定输入IP是否合理。

此外如同我第一周大作业一样，我支持了对界面缩放的支持，没有加分，但是强迫症习惯了┑(￣Д ￣)┍