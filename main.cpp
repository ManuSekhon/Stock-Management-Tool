/**
 * MANUINDER SEKHON
 * C++ College Project
 * 
 * Stock Exchange Management System
 * Allows user to buy and sell company shares at pseudo prices
 */

#define _XOPEN_SOURCE

#include <cstring>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include "helpers.hpp"

using namespace std;

// prototypes
void create_account (void);
void login (int id);
void buy_shares (void);
void sell_shares (void);

// globals
Sqlite3 db = Sqlite3("database.db");

/***********  MAIN  ***********/
int main(int argc, char *argv[])
{
    // ensure db connection
    if (!db.connection())
    {
        cerr << "Error: " << sqlite3_errmsg(db.handle) << '\n';
        return 1;
    }

    // administrator login
    if (argc == 2 && strcmp(argv[1], "admin") == 0)
    {
        Admin admin;
        admin.get_pass("Password: ");
        admin.authenticate();
        admin.login(&db);
    }
    // ensure proper usage
    else if (argc > 2 || (argc == 2 && strcmp(argv[1], "admin") != 0))
    {
        cout << "Usage: main [admin]\n";
        return 1;
    }

    int choice;
    do
    {
        clear();

        // bank menu
        cout << "\033[32m......WELCOME TO BANK......\033[0m\n"
             << "\nManage your Demat account\n"
             << "1. Login\n"
             << "2. Create new account\n"
             << "0. Exit\n"
             << "\nEnter your choice: ";
        
        cin >> choice;
        switch (choice)
        {
            case 1: int id;
                    cout << "\nEnter account number: ";
                    cin >> id;
                    login(id);
                    break;
            
            case 2: create_account();
                    break;

            case 0: break;

            default: cout << "\033[31mEnter valid choice\033[0m\n";
                     usleep(800000);
        }
    }
    while (choice != 0);

    cout << "\033[32mThanks for visiting\033[0m\n";
}
/** MAIN END **/

// Create new user account and store it in database
void create_account(void)
{
    clear();

    cout << "New Account Registration Form\n\n";
    cin >> user;

    // validate user input
    if ((strspn(user.mobile.c_str(), "1234567890") != user.mobile.length()) && (user.mobile.length() != 10))
    {
        cout << "\nPlease enter valid mobile number\n\n";
        wait();
        return;
    }

    try
    {
        // store user in database, with default balance of $10000
        db.execute("INSERT INTO users (name, address, mobile, balance) VALUES ('%s', '%s', '%s', 10000)",
                   user.name.c_str(), user.address.c_str(), user.mobile.c_str());
    }
    catch(...)
    {
        cerr << "Error: " << db.errmsg << '\n';
        wait();
        return;
    }

    // assign account number
    user.acc_num = db.last_row_id();

    // display user info
    clear();
    cout << "Congratulations! Registration successful\n\n"
         << user << '\n';
    wait();

    // end session
    user.logout();
}

// Login into existing user
void login (int acc_num)
{
    clear();

    try
    {
        // get and display user's personal info, if exists
        db.execute(SQL_OUTPUT, "SELECT * FROM users WHERE id=%d", acc_num);
        if (user_stock.acc_num == 0)
        {
            cout << "Account does not exist\n";
            wait();
            return;
        }
        cout << user_stock;
        cout << "Balance       : $" << user_stock.balance << "\n\n";

        // display user's financial info
        cout << setw(25) << left << "ID"
             << setw(25) << left << "SYMBOL" 
             << setw(25) << left << "NUMBER"
             << "TIMESTAMP\n";
        db.execute(SQL_OUTPUT, "SELECT * FROM shares WHERE id=%d", acc_num);
    }
    catch(...)
    {
        cerr << "Error: " << db.errmsg << '\n';
        wait();
        return;
    }

    // prompt
    int choice;
    cout << "\n1. Buy shares\n"
         << "2. Sell shares\n"
         << "0. Logout\n\n"
         << "Enter your choice: ";

    cin >> choice;
    
    if (choice == 1)
    {
        buy_shares();
    }
    else if (choice == 2)
    {
        sell_shares();
    }
    else
    {
        user_stock.logout();
    }
}

// Buy shares of a particular company
void buy_shares (void)
{
    clear();
    double num_shares, cost;
    shares.price = 0.0;

    cout << "Enter company symbol: ";
    cin >> shares.symbol;

    // get share price
    db.execute(SQL_OUTPUT, "SELECT price FROM lookup WHERE symbol = '%s'", shares.symbol.c_str());
    
    // reject invalid company symbol
    if (shares.price == 0.0)
    {
        cout << "Invalid company symbol\n\n";
        wait();
    }
    else
    {
        cout << "Current Price: $" << shares.price << '\n';
        cout << "How many shares do you want to buy? ";
        cin >> num_shares;

        cost = num_shares * shares.price;
        cout << "That's gonna cost you $" << cost << "\n\n";
        wait();

        // ensure user has enough cash
        if (cost <= user_stock.balance && num_shares > 0)
        {
            try
            {
                // increment if user already has that company's stock
                db.execute("UPDATE shares SET number = number + %d WHERE id = %d AND symbol = '%s'",
                            (int)num_shares, user_stock.acc_num, shares.symbol.c_str());
            }
            catch(...)
            {
                // new company's stock bought
                db.execute("INSERT into shares (id, symbol, number) VALUES (%d, '%s', %d)", user_stock.acc_num,
                            shares.symbol.c_str(), (int)num_shares);
            }

            // update current balance
            db.execute("UPDATE users SET balance = %lf WHERE id = %d", user_stock.balance - cost,
                        user_stock.acc_num);
        }
        else
        {
            cout << "Not enough cash\n\n";
            wait();
        }
    }

    login(user_stock.acc_num);
}

// Sell shares of a particular company
void sell_shares (void)
{
    clear();
    int sell_amount;

    cout << "Enter company symbol: ";
    cin >> shares.symbol;

    try
    {
        // get number of shares bought
        db.execute(SQL_OUTPUT, "SELECT number FROM shares WHERE id = %d AND symbol = '%s'", user_stock.acc_num,
                    shares.symbol.c_str());
        
        // get price at which user bought shares
        db.execute(SQL_OUTPUT, "SELECT price FROM lookup WHERE symbol = '%s'", shares.symbol.c_str());

        // display current share value
        double curr_price = shares.current_price();
        cout << "\nYou bought it at $" << shares.price << " per share.\n"
             << "Current price is now $" << curr_price << "\n\n";

        cout << "How many shares of " << shares.symbol << " you want to sell? ";
        cin >> sell_amount;

        // more shares entered than currently available
        if ((size_t)sell_amount > shares.no_of_shares)
        {
            cout << "Sorry! You don't have " << sell_amount << " shares.\n\n";
            wait();
            return;
        }

        double cost = curr_price * (double)sell_amount;
        int shares_left = (int)shares.no_of_shares - sell_amount;

        // all shares sold of that company
        if (shares_left == 0)
        {
            db.execute("DELETE FROM shares WHERE id = %d AND symbol = '%s'", user_stock.acc_num, shares.symbol.c_str());
        }
        else
        {
            db.execute("UPDATE shares SET number = %d WHERE id = %d AND symbol = '%s'", shares_left, user_stock.acc_num,
                        shares.symbol.c_str());
        }
        
        // update user's balance
        db.execute("UPDATE users SET balance = %lf WHERE id = %d", user_stock.balance + cost, user_stock.acc_num);
        cout << "Sold at $" << cost << "\n\n";

    }
    catch(...)
    {
        // invalid company symbol
        cout << "You don't have shares of this company.\n\n";
    }

    wait();
    login(user_stock.acc_num);
}