/**
 * Stock Exchange Management System
 * 
 * Helper Library
 *
 * MANUINDER SEKHON
 * C++ College Project
 */

#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <sqlite3.h>

using namespace std;

#define SQL_OUTPUT 1

// Clear screen
void clear (void);

// Pause execution until return key is pressed
void wait (void);

/**
 * Stores user personal information, which is accessed via external objects stored at the end of this file.
 */
class User
{
    public:
        // personal information
        int acc_num;
        string name;
        string mobile;
        string address;

        User (void);

        // I/O functions
        friend istream& operator >> (istream &is, User &user);
        friend ostream& operator << (ostream &os, User &user);

        // end session
        virtual void logout (void);
};

/**
 * Stores user's financial information
 */
class UserFinance : public User
{
    public:
        double balance;
        double networth;

        UserFinance (void);

        // end session
        void logout (void);
};

/**
 * Stores company's stock information and calculates current (pseudo) market value. 
 */
class Shares
{
    private:
        double value;
    
    public:
        string symbol;
        double price;
        bool row_returned;
        size_t no_of_shares;

        Shares (void);

        // return current share price
        double current_price (void);
};

/**
 * Connect to sqlite database and handles SQL statements.
 */
class Sqlite3
{
    private:
        int conn;

        // sqlite3 callback functions for select statements
        static int callback_users  (void *unused, int count, char **col, char **colName);
        static int callback_shares (void *unused, int count, char **col, char **colName);
        static int callback_lookup (void *unused, int count, char **col, char **colName);
        
        // sets row_returned to true if sql command outputs atleast one row
        static int callback_row_returned (void *unused, int count, char **col, char **colName);
        static int callback_no_of_shares (void *unused, int count, char **col, char **colName);

    public:
        sqlite3 *handle;
        char *errmsg;

        // establish connection with database
        Sqlite3 (char const *db_name);

        // return true if connection is successful
        bool connection (void);

        // get last inserted row id
        int last_row_id (void);

        // execute sql input commands, formatted like printf
        void execute (const char *command, ...) __attribute__((format(printf, 2, 3)));

        // execute sql output commands, formatted like printf
        void execute (int sql_output, const char *command, ...) __attribute__((format(printf, 3, 4)));

        // close connection at end
        ~Sqlite3(void);
};

/**
 * Sign in into administrator account that allows admin to view all
 * registered users. Requires password to login to this account.
 */
class Admin
{
    private:
        string password;
        string admin_pass;
        bool verified;
    
    public:
        Admin (void);

        // input password hiddenly
        void get_pass (const char *prompt);

        // authenticate user
        void authenticate (void);

        // login into admin account
        void login (Sqlite3 *db);
};

// external objects declaration
extern User        user;
extern UserFinance user_stock;
extern Shares      shares;

#endif   // HELPERS_HPP
