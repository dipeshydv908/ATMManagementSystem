#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#define MAX_TRANSACTIONS 100

typedef struct {
    int id;
    char type[20];
    double amount;
    double balanceAfter;
} Transaction;

Transaction transactions[MAX_TRANSACTIONS];

int transactionCount = 0;
int nextTransactionId = 1;
double balance = 5000.00;

/* Read an integer safely */
int readInt(const char *message, int *value) {
    char input[100];
    char *end;
    long number;

    printf("%s", message);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0;

    errno = 0;
    number = strtol(input, &end, 10);

    while (isspace((unsigned char)*end))
        end++;

    if (end == input || *end != '\0' ||
        errno == ERANGE || number < -2147483647L - 1 ||
        number > 2147483647L) {
        printf("Invalid input! Enter a valid integer.\n");
        return 0;
    }

    *value = (int)number;
    return 1;
}

/* Read a positive monetary amount */
int readAmount(double *amount) {
    char input[100];
    char *end;
    char *decimalPoint;
    char *fractionEnd;
    double value;

    printf("Enter amount (Rs.): ");

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0;

    errno = 0;
    value = strtod(input, &end);

    while (isspace((unsigned char)*end))
        end++;

    if (end == input || *end != '\0' ||
        errno == ERANGE || !isfinite(value) || value <= 0.0 ||
        value > 1000000000.0) {
        printf("Invalid amount! Enter a positive amount.\n");
        return 0;
    }

    /* Accept at most two decimal places */
    decimalPoint = strchr(input, '.');
    fractionEnd = decimalPoint == NULL ? NULL : decimalPoint + 1;
    if (fractionEnd != NULL) {
        while (isdigit((unsigned char)*fractionEnd))
            fractionEnd++;
    }
    if (fractionEnd != NULL && fractionEnd - (decimalPoint + 1) > 2) {
        printf("Amount cannot have more than 2 decimal places.\n");
        return 0;
    }

    *amount = value;
    return 1;
}

/* Record a successful banking transaction */
void addTransaction(const char *type, double amount) {
    int i;

    /* Keep the latest 100 records */
    if (transactionCount == MAX_TRANSACTIONS) {
        for (i = 1; i < MAX_TRANSACTIONS; i++)
            transactions[i - 1] = transactions[i];

        transactionCount--;
    }

    transactions[transactionCount].id = nextTransactionId++;
    strcpy(transactions[transactionCount].type, type);
    transactions[transactionCount].amount = amount;
    transactions[transactionCount].balanceAfter = balance;

    transactionCount++;
}

/* Display current balance */
void checkBalance(void) {
    printf("\nCurrent balance: Rs. %.2f\n", balance);
}

/* Deposit cash */
void depositMoney(void) {
    double amount;

    if (!readAmount(&amount))
        return;

    balance += amount;
    addTransaction("Deposit", amount);

    printf("Deposit successful!\n");
    printf("Deposited: Rs. %.2f\n", amount);
    checkBalance();
}

/* Withdraw cash */
void withdrawMoney(void) {
    double amount;

    if (!readAmount(&amount))
        return;

    if (amount > balance) {
        printf("Insufficient balance! Withdrawal denied.\n");
        return;
    }

    balance -= amount;
    addTransaction("Withdrawal", amount);

    printf("Withdrawal successful!\n");
    printf("Withdrawn: Rs. %.2f\n", amount);
    checkBalance();
}

/* Display recent transactions (LIFO order) */
void showHistory(void) {
    int i;

    if (transactionCount == 0) {
        printf("\nNo transactions available.\n");
        return;
    }

    printf("\n========== TRANSACTION HISTORY ==========\n");
    printf("%-8s %-14s %-15s %-15s\n",
           "ID", "Type", "Amount (Rs.)", "Balance (Rs.)");

    for (i = transactionCount - 1; i >= 0; i--) {
        printf("%-8d %-14s Rs. %-11.2f Rs. %.2f\n",
               transactions[i].id,
               transactions[i].type,
               transactions[i].amount,
               transactions[i].balanceAfter);
    }

    printf("=========================================\n");
}

/* Search a transaction by its ID (linear search) */
void searchTransaction(void) {
    int id, i;

    if (transactionCount == 0) {
        printf("\nNo transactions to search.\n");
        return;
    }

    if (!readInt("Enter transaction ID to search: ", &id))
        return;

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].id == id) {
            printf("\nTransaction found!\n");
            printf("ID             : %d\n", transactions[i].id);
            printf("Type           : %s\n", transactions[i].type);
            printf("Amount         : Rs. %.2f\n",
                   transactions[i].amount);
            printf("Balance after  : Rs. %.2f\n",
                   transactions[i].balanceAfter);
            return;
        }
    }

    printf("Transaction ID %d not found in recent history.\n", id);
}

/* Verify user PIN */
int verifyPIN(void) {
    int pin, configuredPin, attempts;

    printf("====================================\n");
    printf("       WELCOME TO ATM SYSTEM\n");
    printf("====================================\n");

    if (!readInt("Set your 4-digit ATM PIN: ", &configuredPin))
        return 0;

    if (configuredPin < 1000 || configuredPin > 9999) {
        printf("PIN must be exactly 4 digits.\n");
        return 0;
    }

    for (attempts = 1; attempts <= 3; attempts++) {
        if (!readInt("Enter your 4-digit PIN: ", &pin)) {
            if (feof(stdin))
                return 0;
            continue;
        }

        if (pin == configuredPin) {
            printf("PIN verified successfully!\n");
            return 1;
        }

        printf("Incorrect PIN! Attempts remaining: %d\n",
               3 - attempts);
    }

    printf("Too many failed attempts. Access denied.\n");
    return 0;
}

/* Main menu */
int main(void) {
    int choice;

    if (!verifyPIN())
        return 0;

    do {
        printf("\n========== ATM MAIN MENU ==========\n");
        printf("1. Check Account Balance\n");
        printf("2. Cash Deposit\n");
        printf("3. Cash Withdrawal\n");
        printf("4. Transaction History\n");
        printf("5. Search Transaction\n");
        printf("6. Exit\n");
        printf("===================================\n");

        if (!readInt("Enter your choice: ", &choice)) {
            if (feof(stdin))
                break;
            continue;
        }

        switch (choice) {
            case 1:
                checkBalance();
                break;

            case 2:
                depositMoney();
                break;

            case 3:
                withdrawMoney();
                break;

            case 4:
                showHistory();
                break;

            case 5:
                searchTransaction();
                break;

            case 6:
                printf("Thank you for using our ATM!\n");
                printf("Exiting safely...\n");
                break;

            default:
                printf("Invalid choice! Select 1 to 6.\n");
        }

        if (feof(stdin))
            break;

    } while (choice != 6);

    return 0;
}