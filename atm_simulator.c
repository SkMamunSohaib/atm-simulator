#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ACCOUNTS 50
#define MAX_NAME_LEN 64
#define MAX_PIN_ATTEMPTS 3
#define HIGH_VALUE_ALERT 25000.0
#define ACCOUNTS_FILE "accounts.txt"
#define TRANSACTIONS_FILE "transactions.txt"

typedef struct {
    int acc_no;
    char name[MAX_NAME_LEN];
    int pin;
    double balance;
    int locked; /* 0 = active, 1 = locked */
} Account;

/* Removes trailing newline/carriage return characters. */
static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

/* Safely reads an integer from stdin; returns 1 on success, 0 on invalid input. */
static int read_int(int *out) {
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return 0;
    }
    char *endptr = NULL;
    long val = strtol(buf, &endptr, 10);
    if (endptr == buf || (*endptr != '\n' && *endptr != '\0' && *endptr != '\r')) {
        return 0;
    }
    *out = (int)val;
    return 1;
}

/* Safely reads a double from stdin; returns 1 on success, 0 on invalid input. */
static int read_double(double *out) {
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return 0;
    }
    char *endptr = NULL;
    double val = strtod(buf, &endptr);
    if (endptr == buf || (*endptr != '\n' && *endptr != '\0' && *endptr != '\r')) {
        return 0;
    }
    *out = val;
    return 1;
}

/* Loads accounts from accounts.txt (format: acc_no|name|pin|balance|locked). */
int load_accounts(const char *filename, Account accounts[], int max_accounts) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    int count = 0;
    char line[256];
    while (count < max_accounts && fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) {
            continue;
        }
        Account acc;
        if (sscanf(line, "%d|%63[^|]|%d|%lf|%d",
                   &acc.acc_no, acc.name, &acc.pin, &acc.balance, &acc.locked) == 5) {
            accounts[count++] = acc;
        }
    }

    fclose(fp);
    return count;
}

/* Saves all accounts back to accounts.txt so balance and lock updates persist. */
int save_accounts(const char *filename, const Account accounts[], int count) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%d|%.2f|%d\n",
                accounts[i].acc_no,
                accounts[i].name,
                accounts[i].pin,
                accounts[i].balance,
                accounts[i].locked);
    }

    fclose(fp);
    return 1;
}

/* Appends a timestamped transaction entry to transactions.txt. */
void log_transaction(int acc_no, const char *type, double amount, double balance_after) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "a");
    if (fp == NULL) {
        printf("Warning: Could not open %s to log transaction.\n", TRANSACTIONS_FILE);
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date_buf[32] = "Unknown Date";
    if (t != NULL) {
        strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M", t);
    }

    fprintf(fp, "%d | %-10s | %.2f | Bal: %.2f | %s\n",
            acc_no, type, amount, balance_after, date_buf);
    fclose(fp);
}

/* Finds an account index by account number; returns -1 if not found. */
int find_account(const Account accounts[], int count, int acc_no) {
    for (int i = 0; i < count; i++) {
        if (accounts[i].acc_no == acc_no) {
            return i;
        }
    }
    return -1;
}

/* Authenticates user with a 3-attempt PIN lock. Returns account index or -1. */
int authenticate(Account accounts[], int count) {
    int acc_no = 0;
    printf("\nEnter Account Number: ");
    if (!read_int(&acc_no)) {
        printf("Invalid account number format.\n");
        return -1;
    }

    int idx = find_account(accounts, count, acc_no);
    if (idx == -1) {
        printf("Account %d not found.\n", acc_no);
        return -1;
    }

    if (accounts[idx].locked) {
        printf("Account %d is locked due to prior failed PIN attempts.\n", acc_no);
        return -1;
    }

    for (int attempt = 1; attempt <= MAX_PIN_ATTEMPTS; attempt++) {
        int entered_pin = 0;
        printf("Enter 4-digit PIN (Attempt %d/%d): ", attempt, MAX_PIN_ATTEMPTS);
        if (!read_int(&entered_pin)) {
            printf("Invalid PIN format.\n");
            continue;
        }

        if (entered_pin == accounts[idx].pin) {
            printf("Welcome, %s!\n", accounts[idx].name);
            return idx;
        }
        printf("Incorrect PIN.\n");
    }

    accounts[idx].locked = 1;
    save_accounts(ACCOUNTS_FILE, accounts, count);
    printf("Too many failed attempts. Account %d is now locked.\n", acc_no);
    return -1;
}

void deposit(Account *acc, Account accounts[], int count) {
    double amount = 0.0;
    printf("Enter deposit amount: ");
    if (!read_double(&amount) || amount <= 0.0) {
        printf("Invalid amount. Deposit must be greater than 0.\n");
        return;
    }

    acc->balance += amount;
    save_accounts(ACCOUNTS_FILE, accounts, count);
    log_transaction(acc->acc_no, "DEPOSIT", amount, acc->balance);
    printf("Deposited %.2f. New balance: %.2f\n", amount, acc->balance);
}

void withdraw(Account *acc, Account accounts[], int count) {
    double amount = 0.0;
    printf("Enter withdrawal amount: ");
    if (!read_double(&amount) || amount <= 0.0) {
        printf("Invalid amount. Withdrawal must be greater than 0.\n");
        return;
    }

    if (amount > acc->balance) {
        printf("Insufficient balance. Available: %.2f\n", acc->balance);
        return;
    }

    if (amount >= HIGH_VALUE_ALERT) {
        printf("[Notice] High-value withdrawal (>= %.2f) flagged in log.\n", HIGH_VALUE_ALERT);
    }

    acc->balance -= amount;
    save_accounts(ACCOUNTS_FILE, accounts, count);
    log_transaction(acc->acc_no, "WITHDRAW", amount, acc->balance);
    printf("Withdrawn %.2f. Remaining balance: %.2f\n", amount, acc->balance);
}

void view_mini_statement(int acc_no) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "r");
    if (fp == NULL) {
        printf("\nNo transaction history found yet.\n");
        return;
    }

    printf("\n--- Mini Statement (Account %d) ---\n", acc_no);
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        int line_acc = 0;
        if (sscanf(line, "%d", &line_acc) == 1 && line_acc == acc_no) {
            printf("  %s", line);
            found = 1;
        }
    }
    if (!found) {
        printf("  (No transactions for this account yet)\n");
    }
    fclose(fp);
}

void change_pin(Account *acc, Account accounts[], int count) {
    int new_pin = 0;
    printf("Enter new 4-digit PIN (1000-9999): ");
    if (!read_int(&new_pin) || new_pin < 1000 || new_pin > 9999) {
        printf("Invalid PIN. Must be a 4-digit number.\n");
        return;
    }

    acc->pin = new_pin;
    save_accounts(ACCOUNTS_FILE, accounts, count);
    printf("PIN updated successfully.\n");
}

void run_session(Account accounts[], int count, int idx) {
    Account *acc = &accounts[idx];
    int choice = 0;

    while (1) {
        printf("\n=== ATM Menu (%s | #%d) ===\n", acc->name, acc->acc_no);
        printf("1. Check Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Mini Statement\n");
        printf("5. Change PIN\n");
        printf("6. Logout\n");
        printf("Enter choice: ");

        if (!read_int(&choice)) {
            printf("Please enter a valid number (1-6).\n");
            continue;
        }

        switch (choice) {
            case 1:
                printf("Current Balance: %.2f\n", acc->balance);
                break;
            case 2:
                deposit(acc, accounts, count);
                break;
            case 3:
                withdraw(acc, accounts, count);
                break;
            case 4:
                view_mini_statement(acc->acc_no);
                break;
            case 5:
                change_pin(acc, accounts, count);
                break;
            case 6:
                printf("Logged out.\n");
                return;
            default:
                printf("Invalid option. Choose 1-6.\n");
        }
    }
}

int main(void) {
    Account accounts[MAX_ACCOUNTS];
    int count = load_accounts(ACCOUNTS_FILE, accounts, MAX_ACCOUNTS);
    if (count <= 0) {
        printf("Error: Could not load accounts from '%s'.\n", ACCOUNTS_FILE);
        printf("Ensure '%s' is in the same folder as the program.\n", ACCOUNTS_FILE);
        return 1;
    }

    while (1) {
        int main_choice = 0;
        printf("\n=== Multi-Account ATM Simulator ===\n");
        printf("1. Login to Account (%d accounts loaded)\n", count);
        printf("2. Exit\n");
        printf("Enter choice: ");

        if (!read_int(&main_choice)) {
            printf("Please enter 1 or 2.\n");
            continue;
        }

        if (main_choice == 1) {
            int idx = authenticate(accounts, count);
            if (idx != -1) {
                run_session(accounts, count, idx);
            }
        } else if (main_choice == 2) {
            printf("Thank you for using the ATM Simulator.\n");
            break;
        } else {
            printf("Invalid choice. Enter 1 or 2.\n");
        }
    }

    return 0;
}