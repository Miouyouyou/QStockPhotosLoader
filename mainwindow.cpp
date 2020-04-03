#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QUrl"
#include "QUrlQuery"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkRequest"
#include "QByteArray"
#include "QtNetwork/QNetworkReply"
#include "QtWebSockets/QWebSocket"
#include "QtWebSockets/QWebSocketServer"
#include "QSettings"
#include "QApplication"
#include "QStandardPaths"

#define LOG(fmt, ...) \
    fprintf(stderr, "[%s:%s:%d] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

enum error_source {
    error_source_invalid,
    error_source_recherche
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , wss(new QWebSocketServer("Pouipux", QWebSocketServer::NonSecureMode, this))
{
    ui->setupUi(this);
    settings_load();
    ui->selectionSources->addItem("Pixabay");
    LOG("Listening : %d", wss->listen(QHostAddress::LocalHost, 8080));
    connect(wss, &QWebSocketServer::newConnection, this, &MainWindow::wss_client_in);
    connect(wss, &QWebSocketServer::closed, this, &MainWindow::wss_client_kicked_out);
}

void MainWindow::settings_save() {
    QString config_filepath =
        QStandardPaths::locate(QStandardPaths::ConfigLocation, "qSPS/config.ini");
    if (config_filepath.isEmpty()) {
        QString const config_dirpath =
            QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        if (config_dirpath.isEmpty()) {
            LOG("Yeah, ok, no config file present and nowhere to save it...");
            LOG("Something's wrong with your system, John.");
        }
        LOG("Dirpath : %s", config_dirpath.toUtf8().data());
        config_filepath = config_dirpath;
        config_filepath.append("/qSPS/config.ini");
        LOG("Config file path : %s", config_filepath.toUtf8().data());
    }
    QSettings settings(config_filepath, QSettings::Format::IniFormat);
    settings.setValue("pixabayKey", ui->textClefAPI->text());
}

MainWindow::~MainWindow()
{
    settings_save();
    int const n_clients = clients_list.length();
    for (int i = 0; i < n_clients; i++) {
        clients_list.at(i)->close();
    }
    wss->close();
    delete ui;
}

void MainWindow::settings_load() {
    QString const config_file =
        QStandardPaths::locate(QStandardPaths::ConfigLocation, "qSPS/config.ini");
    if (config_file.isEmpty()) {
        LOG("No config file. Call the police !");
        return;
    }
    QSettings settings(config_file, QSettings::Format::IniFormat);
    QString key = settings.value("pixabayKey","").toString();
    LOG("Key retrieved : %s", key.toUtf8().data());
    ui->textClefAPI->setText(key);
}

void MainWindow::pixabay_reply(QNetworkReply * const reply) {
    LOG("Reply : %p", reply);
    auto content = reply->readAll();
    LOG("response : %s", content.data());
}

void MainWindow::on_boutonRecherche_clicked()
{
    auto const q =
        this->ui->textRecherche->text();
    QUrl query("https://pixabay.com");
    query.setPath("/api/");
    QUrlQuery params;
    params.addQueryItem("key", ui->textClefAPI->text());
    params.addQueryItem("image_type", "photo");
    params.addQueryItem("q", this->ui->textRecherche->text());
    query.setQuery(params);
    QNetworkRequest request(query);
    connect(
        &nm,
        SIGNAL(finished(QNetworkReply *)),
        this,
        SLOT(pixabay_reply(QNetworkReply *)));
    nm.post(request, QByteArray(""));
    LOG("Requete : %s\n",
        query.toString().toUtf8().data());
}

void MainWindow::wss_client_in() {
    fprintf(stderr, "Got someone in !");
    QWebSocket *pSocket = wss->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &MainWindow::wss_client_sent_text_message);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &MainWindow::wss_client_sent_binary_data);
    connect(pSocket, &QWebSocket::disconnected, this, &MainWindow::wss_client_out);
}

void MainWindow::wss_client_sent_text_message(QString message) {
    fprintf(stderr, "Got a message : %s\n", message.toUtf8().data());
    QWebSocket * client = qobject_cast<QWebSocket *>(sender());
    client->sendTextMessage("Bark bark bark !");
}

static inline void print_buffer(
        QByteArray data,
        char const * __restrict const name)
{
    int const data_length = data.length();
    char const * __restrict const buffer = data.data();
    fprintf(stderr, "%s[%d] = {", name, data_length);
    for (int i = 0; i < data_length; i++) {
        if ((i & 7) == 0)
            fprintf(stderr, "\n\t");
        fprintf(stderr, "%02x, ", buffer[i]);
    }
    fprintf(stderr, "\n};\n");
}

void MainWindow::wss_client_sent_binary_data(QByteArray data) {
    LOG("Got some data\n");
    print_buffer(data, "client_data");
    QWebSocket * client = qobject_cast<QWebSocket *>(sender());
    client->sendTextMessage("WTF ?");

}

void MainWindow::wss_client_kicked_out() {
    LOG("A client got kicked >:)");
}

void MainWindow::wss_client_out() {
    LOG("Someone leaved :C");
}
