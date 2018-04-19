/**
 * Stock Exchange Management System
 * 
 * Helper Library
 *
 * MANUINDER SEKHON
 * C++ College Project
 */

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sqlite3.h>
#include <termios.h>

#include "helpers.hpp"

// External objects definition
User user = User();
UserFinance user_stock = UserFinance();
Shares shares = Shares();

// Default constructors for classes
User::User(void) : acc_num(0) {}

UserFinance::UserFinance(void) : balance(0), networth(0) {}

Shares::Shares(void) : value(0), price(0), row_returned(false), no_of_shares(0) {}

Admin::Admin (void) : admin_pass("admin"), verified(false) {}

// Input user's personal information for new account
istream& operator >> (istream &is, User &user)
{
    cout << "Full name: ";
    cin.ignore();
    getline(cin, user.name);

    cout << "Mobile number: ";
    cin >> user.mobile;

    cout << "Address: ";
    cin.ignore();
    getline(cin, user.address);

    return is;
}

// Output user's personal details
ostream& operator << (ostream &os, User &user)
{
    cout << "Account number: " << user.acc_num << '\n'
         << "Name          : " << user.name << '\n'
         << "Mobile number : " << user.mobile << '\n'
         << "Address       : " << user.address << '\n';

    return os;
}

// End session of currently logged in user
void User::logout (void)
{
    acc_num = 0;
    name = mobile = address = {'\0'};
}
void UserFinance::logout (void)
{
    acc_num = 0;
    name = mobile = address = {'\0'};
    balance = networth = 0.0;
}

// Return (pseudo) current share price, randomly generated.
double Shares::current_price (void)
{
    // seed with system time
    srand48(time(NULL));
    srand(time(NULL));

    int flag[] = {-1,  1,  1, -1, -1, 1, 1, -1,  1, -1, 1, -1, 1,  1, -1, -1, 1, -1};
    double random = drand48() * 20;
    value = price + random * flag[rand() % 18];

    return value;
}

// Establish connection with database
Sqlite3::Sqlite3 (char const *db_name)
{
    conn = sqlite3_open(db_name, &handle);
}

// Return true if connection is successful
inline bool Sqlite3::connection (void)
{
    return conn == SQLITE_OK;
}

// Return newly created user's ID
int Sqlite3::last_row_id (void)
{
    return sqlite3_last_insert_rowid(handle);
}

// Run SQL input statements (i.e., insert, update, delete, ...)
void Sqlite3::execute (const char *command, ...)
{
    char *sql = new char[strlen(command) + 100];
    int return_code;

    // start variable argument list
    va_list ap;
    va_start(ap, command);
    vsprintf(sql, command, ap);

    // make SQL 'update' command throw an error for shares table, if it failed
    if (strncasecmp(sql, "update", 6) == 0 && strstr(sql, "shares") != NULL)
    {
        // turn update into select
        char cmd[200] = "SELECT symbol FROM shares ";
        char *condition = strstr(sql, "WHERE");
        strcat(cmd, condition);

        shares.row_returned = false;
        return_code = sqlite3_exec(handle, cmd, callback_row_returned, NULL, &errmsg);
        if (!shares.row_returned)
        {
            delete[] sql;
            va_end(ap);
            throw SQLITE_ERROR;
        }
    }    

    // execute SQL statement normally
    return_code = sqlite3_exec(handle, sql, NULL, NULL, &errmsg);
    if (return_code != SQLITE_OK)
    {
        delete[] sql;
        va_end(ap);
        throw SQLITE_ERROR;
    }

    // free variable argument list
    delete[] sql;
    va_end(ap);
}

// Run SQL select statements (this function is triggered by SQL_OUTPUT macro)
void Sqlite3::execute (int sql_output, const char *command, ...)
{
    char *sql = new char[strlen(command) + 100];
    int return_code = 1;

    va_list ap;
    va_start(ap, command);
    vsprintf(sql, command, ap);

    // call correct callback function according to table parameters
    if (strstr(sql, "users") != NULL && strstr(sql, "WHERE") != NULL)
    {
        return_code = sqlite3_exec(handle, sql, callback_users, NULL, &errmsg);
    }
    else if (strstr(sql, "shares") != NULL && strstr(sql, "number") != NULL)
    {
        shares.row_returned = false;
        return_code = sqlite3_exec(handle, sql, callback_no_of_shares, NULL, &errmsg);
        if (!shares.row_returned)
        {
            delete[] sql;
            va_end(ap);
            throw SQLITE_ERROR;
        }
    }
    else if (strstr(sql, "shares") != NULL || !strcasecmp(sql, "select * from users"))
    {
        return_code = sqlite3_exec(handle, sql, callback_shares, NULL, &errmsg);
    }
    else if (strstr(sql, "lookup") != NULL)
    {
        return_code = sqlite3_exec(handle, sql, callback_lookup, NULL, &errmsg);
    }

    if (return_code != SQLITE_OK)
    {
        delete[] sql;
        va_end(ap);
        throw SQLITE_ERROR;
    }

    // free variable argument list
    delete[] sql;
    va_end(ap);
}

// Callback function to get user data
int Sqlite3::callback_users (void *unused, int count, char **col, char **colName)
{
    user_stock.acc_num = atoi(col[0]);
    user_stock.name = col[1];
    user_stock.address = col[2];
    user_stock.mobile = col[3];
    user_stock.balance = atof(col[4]);
    
    return 0;
}

// Callback function for to print all shares of logged in user
int Sqlite3::callback_shares (void *unused, int count, char **col, char **colName)
{
    for (int i = 0; i < count; i++)
    {
        cout << setw(25) << left << col[i];
    }
    cout << '\n';
    return 0;
}

// Callback function to find share price of specific company
int Sqlite3::callback_lookup (void *unused, int count, char **col, char **colName)
{
    shares.price = atof(col[0]);
    return 0;
}

// Callback function only called when there is a row available
int Sqlite3::callback_row_returned (void *unused, int count, char **col, char **colName)
{
    shares.row_returned = true;
    return 0;
}

// Callback function only called when user has shares of specific company
int Sqlite3::callback_no_of_shares (void *unused, int count, char **col, char **colName)
{
    shares.no_of_shares = atoi(col[0]);
    shares.row_returned = true;
    return 0;
}

// Close database connection and free variables
Sqlite3::~Sqlite3 (void)
{
    if(connection())
        sqlite3_close(handle);
    
    if (errmsg != NULL)
        sqlite3_free(errmsg);
}

// Input password from terminal hiddenly
// Modified from https://stackoverflow.com/a/30801407
void Admin::get_pass (const char *prompt)
{
    cout << prompt;

    struct termios old_state, new_state;

    // get current terminal state
    tcgetattr(0, &old_state);

    // turn echoing off (i.e., hide input)
    new_state = old_state;
    new_state.c_lflag = new_state.c_lflag & ~ECHO;
    
    // set new terminal state
    tcsetattr(0, TCSAFLUSH, &new_state);

    // read password
    cin >> password;

    // restore old terminal state
    tcsetattr(0, TCSAFLUSH, &old_state);

    cout << "\n";
}

// Check if password entered is correct
void Admin::authenticate (void)
{
    if (password == admin_pass)
    {
        verified = true;
    }
    else
    {
        cerr << "Wrong password\n";
        exit(1);
    }
}

// Display information of all registered users
void Admin::login (Sqlite3 *db)
{
    if (!verified)
    {
        cerr << "You are not authorized to access this account\n";
        exit(1);
    }

    cout << setw(25) << left << "ACCOUNT NUMBER"
         << setw(25) << left << "NAME"
         << setw(25) << left << "ADDRESS"
         << setw(25) << left << "MOBILE"
         << "BALANCE ($)\n";

    db->execute(SQL_OUTPUT, "SELECT * FROM users");
    exit(0);
}

// Clear screen using ANSI escape sequences
void clear (void)
{
    cout << "\033[2J"
         << "\033[0;0H";
}

// Pause execution until return key is pressed
void wait (void)
{
    cout << "Press enter to continue...";
    cin.ignore();
    while (getchar() != '\n');
}