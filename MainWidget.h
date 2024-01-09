#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "jbf293kparser.h"
#include "yiaiparser.h"
#include <QFile>
#include "mqttclient.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>



class QSerialPort;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

    void initSerial();//初始化串口设置
    void initMainUi();//初始化界面操作

    void openSerial();//槽函数-打开串口
    void closeSerial();//槽函数-关闭串口
    void refreshSerial();//槽函数-刷新串口名列表
    void connectbroker();//连接服务器

    void saveDataInTxtFile(QString filepath, QString data);

private:
    QStringList getSerialPortNames();//获取串口名列表
    const QString sf = "82"; //青鸟协议开始标志
    const QString ef = "83"; //青鸟协议结束标志
    const QString bf = "68"; //依爱RS232协议开始标志
    const QString ff = "16"; //依爱RS232协议结束标志
    void setSerialEnable(bool enabled);//串口开启时就不能动ui配置了
    void sendACK();//yiai发送应答包
    void sendInspection();//yiai发送巡检包
    void sendData();//槽函数-发送数据
    void recvData();//槽函数-接收数据
    void processCompletePacket(QByteArray* buffer); //处理不连续数据并解析
    QString mqttIP;  //MQTT连接IP
    int mqttPort;  //MQTT连接端口

    QLineEdit *ipLineEdit;//IP输入框
    QLineEdit *portLineEdit;//端口输入框


private:
    Ui::MainWidget *ui;

    QSerialPort *serialIo;//串口io，一般可以把串口io扔到线程里去，避免阻塞ui

    JBF293KParser jbf293kparser; // 青鸟解析类

    YiaiParse yiaiparse;        //依爱解析类

    MQTTClient mqttclient;

};

#endif // MAINWIDGET_H

