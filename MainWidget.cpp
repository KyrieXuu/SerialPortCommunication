#include "MainWidget.h"
#include "ui_MainWidget.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QListView>

#include <QDebug>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    initSerial();
    initMainUi();

}

MainWidget::~MainWidget()
{
    mqttclient.disconnect();
    delete ui;
}

void MainWidget::initSerial()
{
    //[1]创建串口io对象
    serialIo=new QSerialPort(this);
    //数据接收处理
    connect(serialIo,&QSerialPort::readyRead,this,&MainWidget::recvData);
    //[2]界面初始化
    //注：items的选项值是根据文档中的枚举来写的
    //串口名
    refreshSerial();
    ui->boxPortName->setView(new QListView(this));
    //波特率
    QStringList baudrateList;
    baudrateList<<"1200"<<"2400"<<"4800"<<"9600"<<"19200"<<"38400"<<"57600"<<"115200";
    ui->boxBaudRate->addItems(baudrateList);//添加下拉列表选项
    ui->boxBaudRate->setEditable(true);//串口波特率可编辑
    ui->boxBaudRate->setCurrentText("9600");//界面中初始值
    ui->boxBaudRate->setView(new QListView(this));//该设置是配合qss的，不然item行高设置没效果
    //数据位
    QStringList databitList;
    databitList<<"5"<<"6"<<"7"<<"8";
    ui->boxDataBits->addItems(databitList);
    ui->boxDataBits->setCurrentText("8");
    ui->boxDataBits->setView(new QListView(this));
    //校验位
    QStringList parityList;
    parityList<<"No"<<"Even偶"<<"Odd奇"<<"Space"<<"Mark";
    ui->boxParity->addItems(parityList);
    ui->boxParity->setCurrentText("No");
    ui->boxParity->setView(new QListView(this));
    //停止位
    QStringList stopbitList;
    stopbitList<<"1"<<"1.5"<<"2";
    ui->boxStopBits->addItems(stopbitList);
    ui->boxStopBits->setCurrentText("1");
    ui->boxStopBits->setView(new QListView(this));
    //流控制
    QStringList flowctrlList;
    flowctrlList<<"No"<<"Hardware"<<"Software";
    ui->boxFlowControl->addItems(flowctrlList);
    ui->boxFlowControl->setCurrentText("No");
    ui->boxFlowControl->setView(new QListView(this));

    //IP地址和端口号
    QLabel *ipLabel = new QLabel("IP地址",this);
    QLabel *portLabel = new QLabel("端口号：",this);
    QPushButton *LinkPushButton = new QPushButton(this);
    LinkPushButton->setText("连接");
    ipLineEdit = new QLineEdit(this);
    portLineEdit = new QLineEdit(this);

    //设置输入框属性，限制输入内容为IP地址和数字
    ipLineEdit->setInputMask("000.000.000.000;_");
    portLineEdit->setValidator(new QIntValidator(0,65535,this));

    // 创建一个新的垂直布局，并将标签和输入框添加到其中
    QVBoxLayout *innerLayout = new QVBoxLayout();
    innerLayout->addWidget(ipLabel);
    innerLayout->addWidget(ipLineEdit);
    innerLayout->addWidget(portLabel);
    innerLayout->addWidget(portLineEdit);
    innerLayout->addWidget(LinkPushButton);

    // 在verticalLayout中添加内部布局
    ui->verticalLayout_2->insertLayout(6, innerLayout);
    //点击[连接]按钮
    connect(LinkPushButton,&QPushButton::clicked,this,&MainWidget::connectbroker);
}

void MainWidget::initMainUi()
{
    //点击串口[开启]/[关闭]按钮
    connect(ui->btnOpen,&QPushButton::clicked,this,[this](){
        if(ui->btnOpen->text()=="打开"){
            openSerial();
        }else{
            closeSerial();
        }
    });
    //点击串口[刷新]按钮-刷新串口名列表
    connect(ui->btnRefresh,&QPushButton::clicked,this,&MainWidget::refreshSerial);
    //点击数据[发送]按钮
    connect(ui->btnSend,&QPushButton::clicked,this,&MainWidget::sendData);
}

void MainWidget::openSerial()
{

    mqttclient.publishJSONMessage("testtopic", ui->textSend->toPlainText(), 0);

    const QString portnameStr=ui->boxPortName->currentText();
    if(!portnameStr.isEmpty()){
        QSerialPortInfo info(portnameStr);
        if(info.isNull()){
            qDebug()<<"当前串口繁忙,可能已被占用,请确认后再连接"<<portnameStr;
            return;
        }
        //
        qint32 baudrate=ui->boxBaudRate->currentText().toInt();
        QSerialPort::DataBits databit;
        switch (ui->boxDataBits->currentIndex()) {
        case 0:databit=QSerialPort::Data5; break;
        case 1:databit=QSerialPort::Data6; break;
        case 2:databit=QSerialPort::Data7; break;
        case 3:databit=QSerialPort::Data8; break;
        default:databit=QSerialPort::Data8; break;
        }
        QSerialPort::Parity parity;
        switch (ui->boxParity->currentIndex()) {
        case 0:parity=QSerialPort::NoParity; break;
        case 1:parity=QSerialPort::EvenParity; break;
        case 2:parity=QSerialPort::OddParity; break;
        case 3:parity=QSerialPort::SpaceParity; break;
        case 4:parity=QSerialPort::MarkParity; break;
        default:parity=QSerialPort::NoParity; break;
        }
        QSerialPort::StopBits stopbit;
        switch (ui->boxStopBits->currentIndex()) {
        case 0:stopbit=QSerialPort::OneStop; break;
        case 1:stopbit=QSerialPort::OneAndHalfStop; break;
        case 2:stopbit=QSerialPort::TwoStop; break;
        default:stopbit=QSerialPort::OneStop; break;
        }
        QSerialPort::FlowControl flowcontrol;
        switch (ui->boxFlowControl->currentIndex()) {
        case 0:flowcontrol=QSerialPort::NoFlowControl; break;
        case 1:flowcontrol=QSerialPort::HardwareControl; break;
        case 2:flowcontrol=QSerialPort::SoftwareControl; break;
        default:flowcontrol=QSerialPort::NoFlowControl; break;
        }
        //串口配置设置
        serialIo->setPortName(portnameStr);
        serialIo->setBaudRate(baudrate);
        serialIo->setDataBits(databit);
        serialIo->setParity(parity);
        serialIo->setStopBits(stopbit);
        serialIo->setFlowControl(flowcontrol);//这个我一般没用
        if(serialIo->open(QIODevice::ReadWrite)){
            qDebug()<<"串口已打开,读写模式";
            setSerialEnable(false);//改变ui状态
        }else{
            qDebug()<<"串口打开异常"<<portnameStr<<serialIo->errorString();
            serialIo->clearError();
            setSerialEnable(true);
        }
    }else{
        qDebug()<<"未找到可用串口,请确认串口连接正常后点击刷新";
    }
}

void MainWidget::closeSerial()
{
    serialIo->clear();
    serialIo->close();
    qDebug()<<"串口已关闭";
    setSerialEnable(true);
}

void MainWidget::refreshSerial()
{
    ui->boxPortName->clear();
    ui->boxPortName->addItems(getSerialPortNames());
}

QStringList MainWidget::getSerialPortNames()
{
    QStringList slist;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        //检测是否可用
        if(!info.isNull())
            slist<<info.portName();
    }
    if(slist.isEmpty()){
        qDebug()<<"未找到可用串口,请确认串口连接正常后点击刷新";
    }
    return slist;
}

void MainWidget::setSerialEnable(bool enabled)
{
    //打开成功就false不能再修改配置，关闭状态true可以进行设置
    ui->btnRefresh->setEnabled(enabled);
    ui->btnOpen->setText(enabled?QString("打开"):QString("关闭"));
    //可以把btn和配置分在两个widget里，这样直接设置widget的enable就没这么麻烦了
    ui->boxPortName->setEnabled(enabled);
    ui->boxBaudRate->setEnabled(enabled);
    ui->boxDataBits->setEnabled(enabled);
    ui->boxParity->setEnabled(enabled);
    ui->boxStopBits->setEnabled(enabled);
    ui->boxFlowControl->setEnabled(enabled);
}

void MainWidget::connectbroker()
{
    //连接后不能再更改配置
    ipLineEdit->setReadOnly(true);
    portLineEdit->setReadOnly(true);
    mqttIP = MainWidget::ipLineEdit->text();
    mqttPort = MainWidget::portLineEdit->text().toInt();
    qDebug() << mqttIP << "  " << mqttPort;
    mqttclient.connect(mqttIP, mqttPort);
}

void MainWidget::sendACK(){
    //    const QByteArray send_data=ui->textSend->toPlainText().toUtf8();
    //    if(send_data.size()<=0)
    //        return;
    QByteArray answer = QByteArray::fromStdString(yiaiparse.answerPacks());
    QByteArray hexData = QByteArray::fromHex(answer);
    qDebug()<<answer<<" "<<hexData.toHex();
    if(serialIo->isOpen()){
        qint64 bytesWritten = serialIo->write(hexData.constData(), hexData.size());
        qDebug() << "已发送字节数：" << bytesWritten;
        //        serialIo->write(answer);
        //        qDebug()<<"已发送："<<QString::fromUtf8(answer);
    }else{
        qDebug()<<"发送失败,串口未打开";
        return;
    }
    //Qt新版本默认值是30 000
    if(!serialIo->waitForBytesWritten(30000)){
        qDebug()<<"命令发送异常"<<serialIo->errorString();
                                                serialIo->clearError();
    }
}

void MainWidget::sendInspection(){
    QByteArray inspection = QByteArray::fromStdString(yiaiparse.inspectionInstruction());
    QByteArray hexData = QByteArray::fromHex(inspection);
    qDebug()<<inspection<<" "<<hexData.toHex();
    if(serialIo->isOpen()){
        qint64 bytesWritten = serialIo->write(hexData.constData(), hexData.size());
        qDebug() << "已发送指令字节数：" << bytesWritten;
    }else{
        qDebug()<<"发送失败,串口未打开";
        return;
    }
    //Qt新版本默认值是30 000
    if(!serialIo->waitForBytesWritten(30000)){
        qDebug()<<"命令发送异常"<<serialIo->errorString();
                                                serialIo->clearError();
    }
}
void MainWidget::sendData()
{
    //注意收发的编码问题，我一般只是发命令吗和字节数据，没怎么发字符串，用latin1就行了
//    const QByteArray send_data=ui->textSend->toPlainText().toUtf8();
//    if(send_data.size()<=0)
//        return;
//    if(serialIo->isOpen()){
//        serialIo->write(send_data);
//        qDebug()<<"已发送："<<QString::fromUtf8(send_data);
//    }else{
//        qDebug()<<"发送失败,串口未打开";
//        return;
//    }
//    //Qt新版本默认值是30 000
//    if(!serialIo->waitForBytesWritten(30000)){
//        qDebug()<<"命令发送异常"<<serialIo->errorString();
//        serialIo->clearError();
//    }
    mqttclient.publishJSONMessage("testtopic", ui->textSend->toPlainText(), 0);
}

void MainWidget::recvData()
{
    static QByteArray recv_buffer;
    if (serialIo->bytesAvailable()) {
        //串口收到的数据可能不是连续的，需要的话应该把数据缓存下来再进行协议解析，类似tcp数据处理
        const QByteArray recv_data=serialIo->readAll();
        recv_buffer.append(recv_data);
        //接收发送要一致，如果是处理字节数据，可以把QByteArray当数组一样取下标，或者用data()方法转为char*形式
        processCompletePacket(&recv_buffer);
//        ui->textRecv->append(QString::fromUtf8(recv_data));
//        qDebug()<<"已接收1："<<QString::fromUtf8(recv_data);
        qDebug()<<"已接收2："<<QString::fromUtf8(recv_buffer);
    }
}

void MainWidget::processCompletePacket(QByteArray* buffer){
//    qDebug()<<"接收到"<<buffer;
    qDebug()<<"处理接收到："<<buffer->toHex(' ');
    const QByteArray start_flag = QByteArray::fromHex(sf.toLatin1());  // 青鸟起始标志为 0x82
    const QByteArray end_flag = QByteArray::fromHex(ef.toLatin1());    // 青鸟结束标志为 0x83
    const QByteArray begin_flag = QByteArray::fromHex(bf.toLatin1());  // 依爱起始标志为 0x68
    const QByteArray finish_flag = QByteArray::fromHex(ff.toLatin1());    // 依爱结束标志为 0x16

    // 检查缓冲区中是否存在起始标志和结束标志
    int start_pos = buffer->indexOf(start_flag);
    int end_pos = buffer->indexOf(end_flag);
    int begin_pos = buffer->indexOf(begin_flag);
    QByteArray hexValue = buffer->mid(begin_pos + 1, 1);
//    QByteArray hexValue = buffer[begin_pos+2];
//    char hexValue = buffer->at(begin_pos+1);
//    int length_value = hexValue.toInt(nullptr,16);
    int length_value = hexValue.toHex(' ').toInt(nullptr, 16);
//    int finish_pos = buffer->indexOf(finish_flag);

//    qDebug() << begin_pos << "  " << length_value;

    if (start_pos >= 0 && end_pos >= 0 && end_pos > start_pos) {
        // 提取完整的数据包
        QByteArray complete_packet = buffer->mid(start_pos, end_pos - start_pos + 1);

        // 在这里处理完整的数据包，例如解析、显示等操作
        QString hex_str = complete_packet.toHex(' ');
        string parsed_str = jbf293kparser.parse(hex_str.toUtf8().constData());
        QString resultdata = QString::fromStdString(parsed_str);
        ui->textRecv->append(resultdata);
        qDebug()<<"已接收："<<resultdata;

        // 存储心跳与告警信息
        if (resultdata.indexOf("心跳")>0){
            saveDataInTxtFile("D:/Qt/heartbeat.txt", resultdata);
        }else{
            saveDataInTxtFile("D:/Qt/warning.txt", resultdata);
        }

        // MQTT发送
        mqttclient.publishJSONMessage("testtopic", resultdata, 0);

        // 清除已处理的数据包
        buffer->remove(0, end_pos + 1);

    }else if(begin_pos>=0 && length_value>=0 && buffer->mid(begin_pos, 1)==buffer->mid(begin_pos+3, 1) \
               && buffer->mid(begin_pos+length_value+6-1, 1)==finish_flag){
//        qDebug()<<"进入了yiai解析";
        // 提取完整的数据包
        QByteArray complete_packet = buffer->mid(begin_pos, length_value+6);

        // 在这里处理完整的数据包，例如解析、显示等操作
        QString hex_str = complete_packet.toHex(' ');
        string parsed_str = yiaiparse.parse(hex_str.toUtf8().constData());
        QString resultdata = QString::fromStdString(parsed_str);
        ui->textRecv->append(resultdata);
        qDebug()<<"已接收："<<resultdata;

        // 存储心跳与告警信息
//        if (resultdata.indexOf("心跳")>0){
//            saveDataInTxtFile("D:/Qt/heartbeat.txt", resultdata);
//        }else{
//            saveDataInTxtFile("D:/Qt/warning.txt", resultdata);
//        }
        //打印系统时间
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentTimeString = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
        //存储数据
        saveDataInTxtFile("D:/Qt/yiai2.txt", currentTimeString);
        saveDataInTxtFile("D:/Qt/yiai2.txt", buffer->toHex(' '));
        saveDataInTxtFile("D:/Qt/yiai2.txt", resultdata);
        //在工控机上存储数据
//        saveDataInTxtFile("D:/消防项目/解析数据.txt", currentTimeString);
//        saveDataInTxtFile("D:/消防项目/解析数据.txt", buffer->toHex(' '));
//        saveDataInTxtFile("D:/消防项目/解析数据.txt", resultdata);


        //通过串口发送应答包
        sendACK();
//        //通过串口发送巡检包
//        sendInspection();

        // MQTT发送
        mqttclient.publishJSONMessage("testtopic", resultdata, 0);

        // 清除已处理的数据包
        buffer->remove(0, begin_pos + length_value+6);
//        qDebug()<<"清除后："<<buffer->toHex(' ');
        // 递归调用，继续处理剩余的数据
        processCompletePacket(buffer);
    }else{
        flag++;
        qDebug()<<"标志："<<flag;
        if(flag >2){        //如果连续两次数据错误则清除缓存
            qDebug()<<"循环标志："<<flag;
            //若数据错误，则清除缓存
            buffer->clear();
            qDebug()<<"错误数据清除后："<<buffer->toHex(' ');
            flag = 0;
        }
    }
}

void MainWidget::saveDataInTxtFile(QString filepath, QString data){

    QFile file(filepath);

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream textStream(&file);

        textStream << data << Qt::endl; // 将内容写入文本流

        file.close();
    }

}


