#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include <QFont>
#include <QPalette>
#include <QCalendarWidget>
#include <QTimeEdit>
#include <QStackedWidget>

struct Expense {
    QString description;
    QDateTime expenseDate;
    QString category;
    double amount;

    QString toString() const {
        return description + " - " + expenseDate.toString("yyyy-MM-dd HH:mm:ss") +
               " | Category: " + category + " | Amount: $" + QString::number(amount, 'f', 2);
    }
};

struct User {
    QString username;
    QString password;
    QVector<Expense> expenses;
};

class ExpenseTracker : public QWidget {
    Q_OBJECT

private:
    QStackedWidget* stackedWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *inputLayout;
    QLineEdit *descriptionEdit;
    QCalendarWidget *expenseDateCalendar;
    QTimeEdit *expenseTimeEdit;
    QComboBox *categoryComboBox;
    QLineEdit *amountEdit;
    QPushButton *addExpenseButton;
    QListWidget *expenseList;
    QPushButton *removeExpenseButton;
    QPushButton *sortExpensesButton;
    QPushButton *viewTotalButton;
    QPushButton *logoutButton;

    QVector<User> users;
    User currentUser;

    void setupUI();
    void connectSignalsAndSlots();
    void addExpense();
    void removeExpense();
    void sortExpenses();
    void viewTotal();

public:
    ExpenseTracker(QWidget *parent = nullptr);
    ~ExpenseTracker();
    void saveUserData();
    void loadUserData();
    bool loginUser(const QString& username, const QString& password);
    bool registerUser(const QString& username, const QString& password);
};

ExpenseTracker::ExpenseTracker(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    connectSignalsAndSlots();
}

ExpenseTracker::~ExpenseTracker() {
    saveUserData();
}

void ExpenseTracker::setupUI() {
    mainLayout = new QVBoxLayout(this);
    stackedWidget = new QStackedWidget(this);

    QWidget* loginPage = new QWidget();
    QVBoxLayout* loginLayout = new QVBoxLayout(loginPage);

    QLineEdit* usernameLineEdit = new QLineEdit();
    QLineEdit* passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    QPushButton* loginButton = new QPushButton("Login");

    loginLayout->addWidget(new QLabel("Username:"));
    loginLayout->addWidget(usernameLineEdit);
    loginLayout->addWidget(new QLabel("Password:"));
    loginLayout->addWidget(passwordLineEdit);
    loginLayout->addWidget(loginButton);

    connect(loginButton, &QPushButton::clicked, [=]() {
        if (loginUser(usernameLineEdit->text(), passwordLineEdit->text())) {
            stackedWidget->setCurrentIndex(1); // Switch to the main expense tracker page
        } else {
            QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
        }
    });

    stackedWidget->addWidget(loginPage);

    QWidget* mainPage = new QWidget();
    QVBoxLayout* mainPageLayout = new QVBoxLayout(mainPage);

    inputLayout = new QHBoxLayout();

    descriptionEdit = new QLineEdit(this);
    descriptionEdit->setMinimumWidth(300);

    expenseDateCalendar = new QCalendarWidget(this);
    expenseTimeEdit = new QTimeEdit(this);

    categoryComboBox = new QComboBox(this);
    categoryComboBox->addItems({"Food", "Transportation", "Entertainment", "Utilities", "Others"});

    amountEdit = new QLineEdit(this);
    amountEdit->setPlaceholderText("Amount");

    addExpenseButton = new QPushButton("Add Expense", this);

    QHBoxLayout *expenseDateTimeLayout = new QHBoxLayout();
    expenseDateTimeLayout->addWidget(new QLabel("Expense Date:", this));
    expenseDateTimeLayout->addWidget(expenseDateCalendar);
    expenseDateTimeLayout->addWidget(new QLabel("Expense Time:", this));
    expenseDateTimeLayout->addWidget(expenseTimeEdit);

    // Add widgets to the input layout
    inputLayout->addWidget(new QLabel("Expense Description:", this));
    inputLayout->addWidget(descriptionEdit);
    inputLayout->addLayout(expenseDateTimeLayout);
    inputLayout->addWidget(new QLabel("Category:", this));
    inputLayout->addWidget(categoryComboBox);
    inputLayout->addWidget(new QLabel("Amount:", this));
    inputLayout->addWidget(amountEdit);
    inputLayout->addWidget(addExpenseButton);

    mainPageLayout->addLayout(inputLayout);

    expenseList = new QListWidget(this);
    removeExpenseButton = new QPushButton("Remove Selected Expense", this);
    sortExpensesButton = new QPushButton("Sort Expenses", this);
    viewTotalButton = new QPushButton("View Total", this);
    logoutButton = new QPushButton("Logout", this);

    // Apply modern styling to buttons
    QString buttonStyle = "QPushButton {"
                          "    background-color: %1;"
                          "    color: white;"
                          "    border: none;"
                          "    padding: 10px 20px;"
                          "    border-radius: 5px;"
                          "}"
                          "QPushButton:hover {"
                          "    background-color: %2;"
                          "}";
    removeExpenseButton->setStyleSheet(buttonStyle.arg("#E57373").arg("#EF5350"));
    sortExpensesButton->setStyleSheet(buttonStyle.arg("#81C784").arg("#66BB6A"));
    viewTotalButton->setStyleSheet(buttonStyle.arg("#64B5F6").arg("#42A5F5"));
    logoutButton->setStyleSheet(buttonStyle.arg("#90A4AE").arg("#AEBFC7"));

    mainPageLayout->addWidget(new QLabel("Expense List:", this));
    mainPageLayout->addWidget(expenseList);
    mainPageLayout->addWidget(removeExpenseButton);
    mainPageLayout->addWidget(sortExpensesButton);
    mainPageLayout->addWidget(viewTotalButton);
    mainPageLayout->addWidget(logoutButton);

    // Apply spacing
    mainLayout->setSpacing(20);
    inputLayout->setSpacing(10);

    mainPageLayout->setSpacing(20);
    inputLayout->setSpacing(10);

    mainPageLayout->addLayout(inputLayout);

    stackedWidget->addWidget(mainPage);

    // Apply background color to the main widget
    setStyleSheet("background-color: #f0f0f0;");

    setLayout(mainLayout);
    mainLayout->addWidget(stackedWidget);
}

void ExpenseTracker::connectSignalsAndSlots() {
    connect(addExpenseButton, &QPushButton::clicked, this, &ExpenseTracker::addExpense);
    connect(removeExpenseButton, &QPushButton::clicked, this, &ExpenseTracker::removeExpense);
    connect(sortExpensesButton, &QPushButton::clicked, this, &ExpenseTracker::sortExpenses);
    connect(viewTotalButton, &QPushButton::clicked, this, &ExpenseTracker::viewTotal);
    connect(logoutButton, &QPushButton::clicked, [=]() {
        saveUserData(); // Save user data before logging out
        stackedWidget->setCurrentIndex(0); // Switch to the login page
    });
}

void ExpenseTracker::addExpense() {
    Expense newExpense;
    newExpense.description = descriptionEdit->text();

    if (newExpense.description.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Expense description cannot be empty.");
        return;
    }

    newExpense.expenseDate.setDate(expenseDateCalendar->selectedDate());
    newExpense.expenseDate.setTime(expenseTimeEdit->time());

    newExpense.category = categoryComboBox->currentText();
    newExpense.amount = amountEdit->text().toDouble();

    currentUser.expenses.append(newExpense);
    expenseList->addItem(newExpense.toString());

    descriptionEdit->clear();
    amountEdit->clear();
}

void ExpenseTracker::removeExpense() {
    int selectedRow = expenseList->currentRow();

    if (selectedRow >= 0) {
        currentUser.expenses.remove(selectedRow);
        expenseList->takeItem(selectedRow);
    } else {
        QMessageBox::warning(this, "Warning", "Please select an expense to remove.");
    }
}

void ExpenseTracker::sortExpenses() {
    std::sort(currentUser.expenses.begin(), currentUser.expenses.end(), [](const Expense &a, const Expense &b) {
        return a.expenseDate < b.expenseDate;
    });

    expenseList->clear();
    for (const Expense &expense : currentUser.expenses) {
        expenseList->addItem(expense.toString());
    }
}

void ExpenseTracker::viewTotal() {
    double total = 0.0;
    for (const Expense &expense : currentUser.expenses) {
        total += expense.amount;
    }

    QMessageBox::information(this, "Total Expenses", "Total Expenses: $" + QString::number(total, 'f', 2));
}

void ExpenseTracker::saveUserData() {
    QFile file("userdata.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const User &user : users) {
            out << user.username << ";" << user.password << "\n";
            for (const Expense &expense : user.expenses) {
                out << expense.description << ";" << expense.expenseDate.toString("yyyy-MM-dd HH:mm:ss") << ";"
                    << expense.category << ";" << expense.amount << "\n";
            }
            out << "\n"; // Separate users with an empty line
        }
        file.close();
    }
}

void ExpenseTracker::loadUserData() {
    QFile file("userdata.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString username = in.readLine();
            QString password = in.readLine();

            if (!username.isEmpty() && !password.isEmpty()) {
                User loadedUser;
                loadedUser.username = username;
                loadedUser.password = password;

                while (!in.atEnd()) {
                    QString line = in.readLine();
                    if (line.isEmpty()) {
                        break; // End of user data
                    }

                    QStringList parts = line.split(';');
                    if (parts.size() == 4) {
                        Expense loadedExpense;
                        loadedExpense.description = parts[0];
                        loadedExpense.expenseDate = QDateTime::fromString(parts[1], "yyyy-MM-dd HH:mm:ss");
                        loadedExpense.category = parts[2];
                        loadedExpense.amount = parts[3].toDouble();

                        loadedUser.expenses.append(loadedExpense);
                    }
                }

                users.append(loadedUser);
            }
        }
        file.close();
    }
}

bool ExpenseTracker::loginUser(const QString& username, const QString& password) {
    for (const User &user : users) {
        if (user.username == username && user.password == password) {
            currentUser = user;
            return true;
        }
    }
    return false;
}

bool ExpenseTracker::registerUser(const QString& username, const QString& password) {
    for (const User &user : users) {
        if (user.username == username) {
            return false; // User already exists
        }
    }

    User newUser;
    newUser.username = username;
    newUser.password = password;
    users.append(newUser);
    currentUser = newUser; // Automatically log in the new user
    return true;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ExpenseTracker expenseTracker;
    expenseTracker.setWindowTitle("Modern Expense Tracker");
    expenseTracker.loadUserData(); // Load user data from file
    expenseTracker.show();

    return app.exec();
}
