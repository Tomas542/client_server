#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <winsock2.h>
#include <windows.h>
#include <sys/types.h>

#include <fcntl.h>

#define PORT_FST 9876
#define PORT_SND 9877

int current_server{1};
int _socket[2] = {0, 0};

void connection(int port)
{
    struct sockaddr_in server_addr = {};
    _socket[current_server - 1] = socket(AF_INET, SOCK_STREAM, 0);

    if (_socket[current_server - 1] == -1)
    {
        perror("Socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (::connect(_socket[current_server - 1], (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection to server failed");
        exit(2);
    }

    qDebug() << "Connected to server: " << current_server - 1 << " socket: " << _socket[current_server - 1];
}

void numberCommands(std::string numberBtn)
{
    qDebug() << "Button" << numberBtn << "clicked";
    char buff[128];
    memset(buff, 0, sizeof(buff));
    send(_socket[current_server - 1], numberBtn.c_str(), sizeof(numberBtn.c_str()), 0);

    recv(_socket[current_server - 1], buff, sizeof(buff), 0);
    qDebug() << buff;
}

void switchCommand()
{
    qDebug() << "Switch button clicked";
    current_server = (current_server == 1) ? 2 : 1;

    if (_socket[current_server - 1] == 0)
    {
        connection(current_server == 1 ? PORT_FST : PORT_SND);
    }
}

void closeCommand(QApplication& app)
{
    qDebug() << "Close button clicked";
    for (int i {0}; i < 2; ++i)
    {
        send(_socket[i], "42", sizeof("42"), 0);
        closesocket(_socket[i]);
    }
    app.quit();
}

int main(int argc, char** argv) {
    QApplication guiApp(argc, argv);

    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0)
    {
        qDebug() << "Error";
        exit(1);
    }

    QWidget mainWindow;
    mainWindow.setWindowTitle("Server application");
    mainWindow.setGeometry(100, 100, 400, 200);

    QHBoxLayout *numberButtonsLayout = new QHBoxLayout;
    QHBoxLayout *funcButtonsLayout = new QHBoxLayout;

    QPushButton button1("1");
    QPushButton button2("2");
    QPushButton switchButton("Switch");
    QPushButton closeButton("Close");

    numberButtonsLayout->addWidget(&button1);
    numberButtonsLayout->addWidget(&button2);
    funcButtonsLayout->addWidget(&switchButton);
    funcButtonsLayout->addWidget(&closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(&mainWindow);
    mainLayout->addLayout(numberButtonsLayout);
    mainLayout->addLayout(funcButtonsLayout);

    mainWindow.setLayout(mainLayout);
    mainWindow.show();

    connection(PORT_FST); // Initial connection

    QObject::connect(&button1, &QPushButton::clicked, std::bind(&numberCommands, "1"));

    QObject::connect(&button2, &QPushButton::clicked, std::bind(&numberCommands, "2"));

    QObject::connect(&switchButton, &QPushButton::clicked, std::bind(&switchCommand));

    QObject::connect(&closeButton, &QPushButton::clicked, std::bind(&closeCommand, std::ref(guiApp)));

    return guiApp.exec();
}
