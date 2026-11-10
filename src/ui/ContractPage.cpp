#include "ContractPage.h"

#include "TextUtils.h"
#include "Validator.h"

#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLocale>
#include <QStringList>
#include <QFileInfo>
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QTextDocument>
#include <QVBoxLayout>

using namespace std;

namespace {
QString value(const string& s) { return QString::fromStdString(s); }
QString money(int64_t n) { return QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(qlonglong(n)) + QString::fromUtf8(" đ"); }
QString trimText(QLineEdit* edit) { return edit->text().trimmed(); }
QString editStyle() {
    return "QDialog { background:#F4F7FF; } QLabel { color:#243962; } "
           "QLineEdit,QComboBox { background:white; color:#153568; border:1px solid #CFDBEF; "
           "border-radius:7px; padding:8px; min-height:20px; } "
           "QPushButton { background:#2F6BFF; color:white; border-radius:8px; padding:8px 16px; }";
}
}

ContractPage::ContractPage(Vector<Student*>& students, Vector<Tutor*>& tutors,
                           Vector<TutoringClass*>& classes, QWidget* parent)
    : QWidget(parent), students_(students), tutors_(tutors), classes_(classes),
      service_(students, tutors, classes) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 24, 26, 22);
    root->setSpacing(16);
    auto* title = new QLabel(QString::fromUtf8("Quản lý hợp đồng"), this);
    title->setStyleSheet("font-size:25px;font-weight:800;color:#142E72;");
    root->addWidget(title);
    auto* subtitle = new QLabel(QString::fromUtf8(
        "Hợp đồng lớp học, học viên và gia sư · Thu phí và xuất hợp đồng PDF"), this);
    subtitle->setStyleSheet("color:#6578A3;font-size:13px;");
    root->addWidget(subtitle);

    auto* filters = new QFrame(this);
    filters->setStyleSheet("QFrame { background:#FFFFFF;border:1px solid #DBE5F6;border-radius:12px; }");
    auto* filterLayout = new QHBoxLayout(filters);
    filterLayout->setContentsMargins(18, 12, 18, 12);
    search_ = new QLineEdit(filters);
    search_->setPlaceholderText(QString::fromUtf8("Mã hợp đồng, mã đối tượng, tên học viên/gia sư..."));
    search_->setMinimumHeight(37);
    paymentState_ = new QComboBox(filters);
    paymentState_->addItems({QString::fromUtf8("Tất cả"),QString::fromUtf8("Chưa thu"),
                             QString::fromUtf8("Thu một phần"),QString::fromUtf8("Đã thu đủ"),
                             QString::fromUtf8("Đã hủy")});
    paymentState_->setMinimumWidth(160);
    paymentState_->setMinimumHeight(37);
    auto* reset=new QPushButton(QString::fromUtf8("Xóa lọc"),filters);
    reset->setMinimumHeight(37);
    filterLayout->addWidget(search_,1);
    filterLayout->addWidget(paymentState_);
    filterLayout->addWidget(reset);
    root->addWidget(filters);

    tabs_=new QTabWidget(this);
    tabs_->addTab(createTable("CLASS"),QString::fromUtf8("Hợp đồng lớp học"));
    tabs_->addTab(createTable("STUDENT"),QString::fromUtf8("Hợp đồng học viên"));
    tabs_->addTab(createTable("TUTOR"),QString::fromUtf8("Hợp đồng gia sư"));
    root->addWidget(tabs_,1);

    auto* actions=new QHBoxLayout;
    actions->setSpacing(8);
    auto button=[this,actions](const QString& name) {
        auto* b=new QPushButton(name,this);
        b->setMinimumHeight(39);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet("QPushButton { background:#2F6BFF;color:#fff;border:0;border-radius:8px;padding:5px 12px;font-weight:600; }"
                         "QPushButton:hover { background:#1450D6; }QPushButton:disabled{background:#C5D1E4;}");
        actions->addWidget(b);
        return b;
    };
    add_=button(QString::fromUtf8("+ Tạo hợp đồng"));
    edit_=button(QString::fromUtf8("Sửa"));
    detail_=button(QString::fromUtf8("Chi tiết"));
    pay_=button(QString::fromUtf8("Ghi nhận thanh toán"));
    pdf_=button(QString::fromUtf8("Xuất PDF"));
    cancel_=button(QString::fromUtf8("Hủy hiệu lực"));
    remove_=button(QString::fromUtf8("Xóa"));
    root->addLayout(actions);
    summary_=new QLabel(this);
    summary_->setStyleSheet("font-size:12px;color:#516A9B;");
    root->addWidget(summary_);

    connect(tabs_,&QTabWidget::currentChanged,this,[this] { renderTables(); });
    connect(search_,&QLineEdit::textChanged,this,[this] { renderTables(); });
    connect(paymentState_,&QComboBox::currentIndexChanged,this,[this] { renderTables(); });
    connect(reset,&QPushButton::clicked,this,[this] { search_->clear();paymentState_->setCurrentIndex(0); });
    connect(add_,&QPushButton::clicked,this,[this] { editContract(true); });
    connect(edit_,&QPushButton::clicked,this,[this] { editContract(false); });
    connect(detail_,&QPushButton::clicked,this,[this] { viewDetails(); });
    connect(pay_,&QPushButton::clicked,this,[this] { recordPayment(); });
    connect(pdf_,&QPushButton::clicked,this,[this] { exportPdf(); });
    connect(cancel_,&QPushButton::clicked,this,[this] { cancelContract(); });
    connect(remove_,&QPushButton::clicked,this,[this] { deleteContract(); });
    refresh();
}

QTableWidget* ContractPage::createTable(const QString& type) {
    auto* table=new QTableWidget(this);
    table->setObjectName("contractTable_"+type);
    if(type=="CLASS") tables_[0]=table;
    else if(type=="STUDENT") tables_[1]=table;
    else tables_[2]=table;
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({QString::fromUtf8("Mã hợp đồng"),QString::fromUtf8("Đối tượng"),
        QString::fromUtf8("Họ tên / Lớp"),QString::fromUtf8("Ngày lập"),
        QString::fromUtf8("Học phí/giờ"),QString::fromUtf8("Phí dịch vụ"),
        QString::fromUtf8("Đã thu"),QString::fromUtf8("Còn lại"),QString::fromUtf8("Trạng thái")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(false);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(38);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    table->setStyleSheet("QTableWidget { background:#fff;alternate-background-color:#F4F7FF;color:#18356B;"
                         "border:1px solid #DBE5F6;border-radius:8px;}"
                         "QTableWidget::item:selected {background:#DDEAFF;color:#0A2673;}"
                         "QHeaderView::section {background:#E7EFFF;color:#173878;padding:8px;font-weight:700;}");
    connect(table,&QTableWidget::cellDoubleClicked,this,[this](int,int){viewDetails();});
    connect(table,&QTableWidget::itemSelectionChanged,this,[this]{
        const auto* selectedRecord=selected();
        edit_->setEnabled(selectedRecord && selectedRecord->active);
        pay_->setEnabled(selectedRecord && selectedRecord->active && service_.outstanding(selectedRecord->id)>0);
    });
    return table;
}

QString ContractPage::entityName(const ContractRecord& record) const {
    if (record.type=="STUDENT") {
        for (size_t i=0;i<students_.size();++i)
            if (students_[i]->getPersonID()==record.referenceId) return value(students_[i]->getFullName());
    } else if (record.type=="TUTOR") {
        for (size_t i=0;i<tutors_.size();++i)
            if (tutors_[i]->getPersonID()==record.referenceId) return value(tutors_[i]->getFullName());
    } else {
        for (size_t i=0;i<classes_.size();++i)
            if (classes_[i]->getClassID()==record.referenceId) {
                QString sid=value(classes_[i]->getStudentID());
                QString tid=value(classes_[i]->getTutorID());
                return QString::fromUtf8("Lớp %1 · %2 / %3").arg(value(record.referenceId),sid,tid);
            }
    }
    return QString::fromUtf8("Không tìm thấy đối tượng");
}

QString ContractPage::contractTypeTitle(const string& type) const {
    if(type=="CLASS") return QString::fromUtf8("Hợp đồng lớp học");
    if(type=="STUDENT") return QString::fromUtf8("Hợp đồng dịch vụ học viên");
    return QString::fromUtf8("Hợp đồng cộng tác viên gia sư");
}

const ContractRecord* ContractPage::selected() const {
    const int tab=tabs_->currentIndex();
    if(tab<0||tab>2||!tables_[tab]||tables_[tab]->currentRow()<0) return nullptr;
    auto* item=tables_[tab]->item(tables_[tab]->currentRow(),0);
    return item?service_.find(item->data(Qt::UserRole).toString().toStdString()):nullptr;
}

void ContractPage::renderTables() {
    const QStringList types={"CLASS","STUDENT","TUTOR"};
    const QString q=search_->text().trimmed();
    int visibleCount=0;
    for(int tab=0;tab<3;++tab) {
        auto* table=tables_[tab];
        if(!table) continue;
        const QString oldId=table->currentItem() && table->item(table->currentRow(),0)
            ? table->item(table->currentRow(),0)->text():QString();
        table->setRowCount(0);
        for(size_t i=0;i<service_.contracts().size();++i) {
            const auto& c=service_.contracts()[i];
            if(value(c.type)!=types[tab]) continue;
            const QString party=entityName(c);
            if(!q.isEmpty() && TextUtils::normalize(c.id).find(TextUtils::normalize(q.toStdString()))==string::npos &&
               TextUtils::normalize(c.referenceId).find(TextUtils::normalize(q.toStdString()))==string::npos &&
               TextUtils::normalize(party.toStdString()).find(TextUtils::normalize(q.toStdString()))==string::npos) continue;
            const int64_t paid=service_.totalPaid(c.id);
            const int filter=paymentState_->currentIndex();
            if((filter==1 && (!c.active || paid!=0)) ||
               (filter==2 && (!c.active || paid==0 || paid==c.commission)) ||
               (filter==3 && (!c.active || paid!=c.commission)) ||
               (filter==4 && c.active)) continue;
            const int row=table->rowCount();
            table->insertRow(row);
            const QStringList items={value(c.id),value(c.referenceId),party,value(c.issuedDate),
                c.type=="CLASS"?money(c.hourlyRate):QString::fromUtf8("—"),money(c.commission),
                money(paid),money(c.commission-paid),
                !c.active?QString::fromUtf8("Đã hủy"):
                    (paid>=c.commission?QString::fromUtf8("Đã thu đủ"):
                     (paid==0?QString::fromUtf8("Chưa thu"):QString::fromUtf8("Thu một phần")))};
            for(int col=0;col<items.size();++col) table->setItem(row,col,new QTableWidgetItem(items[col]));
            table->item(row,0)->setData(Qt::UserRole,value(c.id));
            if(items[0]==oldId) table->selectRow(row);
            ++visibleCount;
        }
        table->clearSelection();
        table->setCurrentCell(-1,-1);
        for(int row=0;row<table->rowCount();++row) {
            if(table->item(row,0)->text()==oldId) {
                table->selectRow(row);
                break;
            }
        }
    }
    summary_->setText(QString::fromUtf8("Tổng: %1 hợp đồng · Đã thu: %2 · Còn phải thu: %3")
        .arg(qlonglong(service_.contracts().size()))
        .arg(money(service_.totalRevenue()))
        .arg([this] {
            int64_t total=0;
            for(size_t i=0;i<service_.contracts().size();++i) {
                const auto& c=service_.contracts()[i];
                if(c.active) total+=service_.outstanding(c.id);
            }
            return money(total);
        }()));
    const auto* c=selected();
    edit_->setEnabled(c && c->active);
    detail_->setEnabled(c);
    pdf_->setEnabled(c);
    pay_->setEnabled(c && c->active && service_.outstanding(c->id)>0);
    cancel_->setEnabled(c && c->active);
    remove_->setEnabled(c);
}

void ContractPage::refresh() {
    string error;
    if (!service_.reload(error)) {
        QMessageBox::critical(this,QString::fromUtf8("Lỗi dữ liệu hợp đồng"),value(error));
        add_->setEnabled(false);
        return;
    }
    add_->setEnabled(true);
    renderTables();
}
int ContractPage::contractCount() const { return int(service_.contracts().size()); }
int64_t ContractPage::totalPaid() const { return service_.totalRevenue(); }
int64_t ContractPage::paidThisMonth(int year,int month) const { return service_.revenueByMonth(year,month); }
int64_t ContractPage::monthlyRevenue(int year,int month) const { return service_.revenueByMonth(year,month); }

void ContractPage::editContract(bool creating) {
    const ContractRecord* original=creating?nullptr:selected();
    if (!creating && !original) return;
    const int tab=tabs_->currentIndex();
    const string type=tab==0?"CLASS":tab==1?"STUDENT":"TUTOR";
    ContractRecord record=original?*original:ContractRecord{};
    if(creating) {record.type=type;record.id=ContractService::makeNextId(service_.contracts(),type);}
    QDialog dialog(this);
    dialog.setWindowTitle(creating?QString::fromUtf8("Tạo ")+contractTypeTitle(type)
                       :QString::fromUtf8("Chỉnh sửa ")+value(record.id));
    dialog.setStyleSheet(editStyle());
    dialog.setMinimumWidth(530);
    auto* form=new QFormLayout(&dialog);
    auto* idField=new QLabel(value(record.id));
    form->addRow(QString::fromUtf8("Mã hợp đồng:"),idField);
    auto* entity=new QComboBox(&dialog);
    if(type=="CLASS") {
        for(size_t i=0;i<classes_.size();++i) {
            const auto* c=classes_[i];
            if(c->getTutorID().empty()) continue;
            if(creating && service_.hasReference("CLASS",c->getClassID())) continue;
            entity->addItem(QString::fromUtf8("%1 · %2 · HS %3 · GS %4")
                .arg(value(c->getClassID()),value(c->getSubject()),value(c->getStudentID()),value(c->getTutorID())),
                value(c->getClassID()));
        }
    } else if(type=="STUDENT") {
        for(size_t i=0;i<students_.size();++i) entity->addItem(
            value(students_[i]->getPersonID())+" · "+value(students_[i]->getFullName()),
            value(students_[i]->getPersonID()));
    } else {
        for(size_t i=0;i<tutors_.size();++i) entity->addItem(
            value(tutors_[i]->getPersonID())+" · "+value(tutors_[i]->getFullName()),
            value(tutors_[i]->getPersonID()));
    }
    if(!creating) {entity->setCurrentIndex(entity->findData(value(record.referenceId)));entity->setEnabled(false);}
    form->addRow(QString::fromUtf8("Đối tượng:"),entity);
    auto* issued=new QLineEdit(&dialog);
    issued->setPlaceholderText("dd/MM/yyyy");
    issued->setMaxLength(10);
    issued->setText(creating?QDate::currentDate().toString("dd/MM/yyyy"):value(record.issuedDate));
    form->addRow(QString::fromUtf8("Ngày lập (*):"),issued);
    auto* rate=new QLineEdit(&dialog);
    rate->setReadOnly(true);
    rate->setText(type=="CLASS"?QString::number(record.hourlyRate):"0");
    if(type=="CLASS") {
        auto syncRate=[this,entity,rate]() {
            const QString id=entity->currentData().toString();
            for(size_t i=0;i<classes_.size();++i) if(value(classes_[i]->getClassID())==id) {
                rate->setText(value(classes_[i]->getRatePerHour()));return;
            }
            rate->setText("0");
        };
        connect(entity,&QComboBox::currentIndexChanged,&dialog,syncRate);
        if(creating) syncRate();
        form->addRow(QString::fromUtf8("Học phí/giờ (VNĐ):"),rate);
    }
    auto* commission=new QLineEdit(&dialog);
    commission->setPlaceholderText(QString::fromUtf8("Ví dụ: 400000"));
    commission->setText(QString::number(record.commission));
    commission->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,15}"),commission));
    form->addRow(QString::fromUtf8("Phí dịch vụ/hoa hồng (VNĐ):"),commission);
    auto* notes=new QLineEdit(value(record.notes),&dialog);
    notes->setPlaceholderText(QString::fromUtf8("Ghi chú (không bắt buộc)"));
    form->addRow(QString::fromUtf8("Ghi chú:"),notes);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);
    buttons->button(QDialogButtonBox::Save)->setText(QString::fromUtf8("Lưu hợp đồng"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("Bỏ qua"));
    form->addRow(buttons);
    connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::accepted,&dialog,[this,&dialog,creating,&record,entity,issued,rate,commission,notes]() mutable {
        bool okCommission=false,okRate=false;
        const qlonglong fee=commission->text().toLongLong(&okCommission);
        const qlonglong hourly=rate->text().toLongLong(&okRate);
        if(entity->currentIndex()<0 || !Validator::isValidDate(trimText(issued).toStdString()) ||
           !okCommission || fee<0 || !okRate || hourly<0) {
            QMessageBox::warning(&dialog,QString::fromUtf8("Dữ liệu chưa hợp lệ"),
                QString::fromUtf8("Chọn đối tượng, nhập ngày dd/MM/yyyy hợp lệ và số tiền không âm."));
            return;
        }
        record.referenceId=entity->currentData().toString().toStdString();
        record.issuedDate=trimText(issued).toStdString();
        record.hourlyRate=hourly;
        record.commission=fee;
        record.notes=trimText(notes).toStdString();
        string error;
        bool ok=creating?service_.create(record,error):service_.update(record.id,record,error);
        if(!ok) {QMessageBox::warning(&dialog,QString::fromUtf8("Không thể lưu"),value(error));return;}
        dialog.accept();
        renderTables();
    });
    dialog.exec();
}

void ContractPage::recordPayment() {
    const auto* c=selected();
    if(!c || !c->active || service_.outstanding(c->id)<=0) return;
    const string contractId=c->id;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Ghi nhận thanh toán: ")+value(contractId));
    dialog.setStyleSheet(editStyle());dialog.setMinimumWidth(460);
    auto* layout=new QFormLayout(&dialog);
    layout->addRow(QString::fromUtf8("Còn phải thu:"),new QLabel(money(service_.outstanding(contractId)),&dialog));
    auto* date=new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy"),&dialog);
    date->setMaxLength(10);date->setPlaceholderText("dd/MM/yyyy");
    layout->addRow(QString::fromUtf8("Ngày thanh toán (*):"),date);
    auto* amount=new QLineEdit(QString::number(service_.outstanding(contractId)),&dialog);
    amount->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,17}"),amount));
    layout->addRow(QString::fromUtf8("Số tiền (VNĐ) (*):"),amount);
    auto* memo=new QLineEdit(&dialog);memo->setPlaceholderText(QString::fromUtf8("Tiền mặt, chuyển khoản, số tham chiếu..."));
    layout->addRow(QString::fromUtf8("Ghi chú:"),memo);
    auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);
    layout->addRow(actions);
    connect(actions,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    connect(actions,&QDialogButtonBox::accepted,&dialog,[this,&dialog,contractId,date,amount,memo] {
        bool parsed=false;qlonglong moneyPaid=trimText(amount).toLongLong(&parsed);
        if(!parsed || moneyPaid<=0) {
            QMessageBox::warning(&dialog,QString::fromUtf8("Lỗi"),QString::fromUtf8("Số tiền phải lớn hơn 0."));return;
        }
        PaymentRecord payment;
        payment.contractId=contractId;payment.amount=moneyPaid;
        payment.paidDate=trimText(date).toStdString();payment.notes=trimText(memo).toStdString();
        string error;
        if(!service_.addPayment(payment,error)) {
            QMessageBox::warning(&dialog,QString::fromUtf8("Không thể thanh toán"),value(error));return;
        }
        dialog.accept();renderTables();
    });
    dialog.exec();
}

void ContractPage::viewDetails() {
    const auto* c=selected();if(!c)return;
    const ContractRecord info=*c;
    QString message=QString::fromUtf8("%1\n\nMã hợp đồng: %2\nĐối tượng: %3 — %4\n"
       "Ngày lập: %5\nHọc phí/giờ (nếu có): %6\nPhí dịch vụ: %7\nĐã thu: %8\n"
       "Còn phải thu: %9\nTình trạng: %10\nGhi chú: %11\n\nLỊCH SỬ THANH TOÁN");
    for (const auto& item : QStringList{
        contractTypeTitle(info.type),value(info.id),value(info.referenceId),entityName(info),
        value(info.issuedDate),money(info.hourlyRate),money(info.commission),
        money(service_.totalPaid(info.id)),money(service_.outstanding(info.id)),
        info.active?QString::fromUtf8("Có hiệu lực"):QString::fromUtf8("Đã hủy"),value(info.notes)})
        message=message.arg(item);
    bool has=false;
    for(size_t i=0;i<service_.payments().size();++i) {
        const auto& p=service_.payments()[i];if(p.contractId!=info.id)continue;
        has=true;message+=QString::fromUtf8("\n• %1 · %2 · %3 · %4")
          .arg(value(p.id),value(p.paidDate),money(p.amount),value(p.notes));
    }
    if(!has)message+=QString::fromUtf8("\nChưa ghi nhận khoản thanh toán.");
    QMessageBox dialog(this);dialog.setWindowTitle(QString::fromUtf8("Chi tiết hợp đồng"));
    dialog.setText(message);dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
    dialog.setIcon(QMessageBox::Information);dialog.exec();
}

void ContractPage::exportPdf() {
    const auto* c=selected();if(!c)return;
    const ContractRecord info=*c;
    const QString name=QFileDialog::getSaveFileName(this,QString::fromUtf8("Xuất hợp đồng PDF"),
        QString::fromUtf8("HopDong_%1.pdf").arg(value(info.id)),QString::fromUtf8("PDF (*.pdf)"));
    if(name.isEmpty())return;
    const QString title=contractTypeTitle(info.type).toHtmlEscaped();
    const QString party=entityName(info).toHtmlEscaped();
    const QString notes=value(info.notes).toHtmlEscaped();
    QString html=QString::fromUtf8(
        "<html><head><meta charset='utf-8'/><style>body{font-family:'Arial';font-size:11pt;color:#172b4d;}"
        "h1{text-align:center;font-size:19pt;color:#0C338C;}h2{font-size:13pt;color:#164B9E;}"
        "table{border-collapse:collapse;width:100%;margin-top:12px;}td{border:1px solid #ccc;padding:9px;}"
        "</style></head><body><h1>%1</h1>"
        "<p style='text-align:center'>Mã: <b>%2</b> · Ngày lập: %3</p>"
        "<h2>1. Thông tin đối tượng</h2><table>"
        "<tr><td>Mã đối tượng</td><td>%4</td></tr><tr><td>Họ tên / Lớp</td><td>%5</td></tr>"
        "<tr><td>Học phí/giờ (nếu áp dụng)</td><td>%6</td></tr>"
        "<tr><td>Phí dịch vụ/hoa hồng đã thống nhất</td><td>%7</td></tr>"
        "<tr><td>Phí đã thu (theo sổ quản lý)</td><td>%8</td></tr>"
        "</table>"
        "<h2>2. Thỏa thuận</h2><p>Phí dịch vụ và phương thức thanh toán thực hiện "
        "theo thỏa thuận riêng giữa trung tâm và các bên. Phí còn phải thu: <b>%9</b>.</p>"
        "<p>Ghi chú: %10</p>"
        "<h2>3. Xác nhận</h2><p>Hai bên xác nhận thông tin và điều khoản thực tế "
        "trước khi ký. Tài liệu mẫu này không tự thay thế hợp đồng pháp lý đã ký.</p>"
        "<table><tr><td style='height:80px;text-align:center'>ĐẠI DIỆN TRUNG TÂM<br/>(Ký, ghi rõ họ tên)</td>"
        "<td style='text-align:center'>BÊN LIÊN QUAN<br/>(Ký, ghi rõ họ tên)</td></tr></table>"
        "</body></html>");
    for (const auto& item : QStringList{title,value(info.id).toHtmlEscaped(),
             value(info.issuedDate).toHtmlEscaped(),value(info.referenceId).toHtmlEscaped(),
             party,money(info.hourlyRate),money(info.commission),
             money(service_.totalPaid(info.id)),money(service_.outstanding(info.id)),notes})
        html=html.arg(item);
    {
        QPdfWriter writer(name);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setResolution(200);
        writer.setTitle(contractTypeTitle(info.type)+" "+value(info.id));
        QTextDocument document;
        document.setHtml(html);
        document.print(&writer);
    } // QPdfWriter is finalized on destruction; only inspect file afterwards.
    // PDF is written by Qt; verify a non-empty output before telling the user.
    if (!QFileInfo(name).exists() || QFileInfo(name).size()==0) {
        QMessageBox::warning(this,QString::fromUtf8("Xuất PDF thất bại"),
            QString::fromUtf8("Không thể tạo file PDF tại đường dẫn đã chọn."));
        return;
    }
    QMessageBox::information(this,QString::fromUtf8("Đã xuất"),
        QString::fromUtf8("Đã tạo bản mẫu hợp đồng tại:\n")+name);
}

void ContractPage::cancelContract() {
    const auto* c=selected();if(!c || !c->active)return;
    const string id=c->id;
    if(QMessageBox::question(this,QString::fromUtf8("Hủy hiệu lực"),
       QString::fromUtf8("Hủy hiệu lực hợp đồng %1? Lịch sử thanh toán được giữ nguyên.").arg(value(id)))
       !=QMessageBox::Yes)return;
    string error;
    if(!service_.cancel(id,error)) QMessageBox::warning(this,QString::fromUtf8("Lỗi"),value(error));
    renderTables();
}

void ContractPage::deleteContract() {
    const auto* c=selected();if(!c)return;
    const string id=c->id;
    if(QMessageBox::question(this,QString::fromUtf8("Xác nhận xóa"),
       QString::fromUtf8("Xóa hợp đồng %1? Không thể xóa hợp đồng có lịch sử thanh toán.").arg(value(id)))
       !=QMessageBox::Yes)return;
    string error;
    if(!service_.remove(id,error)) QMessageBox::warning(this,QString::fromUtf8("Không thể xóa"),value(error));
    renderTables();
}
