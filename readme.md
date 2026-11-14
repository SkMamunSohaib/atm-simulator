# Multi-Account ATM Simulator in C

A modular, file-backed ATM simulator written in C11 for **JIS College of Engineering — CAA Project (Level 1)**. It demonstrates `struct`-based account management, persistent file I/O, input validation, 3-attempt PIN locking, and timestamped transaction logging.

## Project Files

| File | Purpose |
| :--- | :--- |
| `atm_simulator.c` | Main C11 source code with modular banking and file-handling functions |
| `accounts.txt` | Pipe-delimited account database (`acc_no\|name\|pin\|balance\|locked`) |
| `transactions.txt` | Auto-generated log storing timestamped deposits and withdrawals |

## Key Features

- **Multi-Account Login:** Loads up to 50 accounts from `accounts.txt` into an array of `Account` structs.
- **3-Attempt PIN Lock:** Automatically locks an account (`locked = 1`) after 3 consecutive wrong PIN entries and persists the lock to `accounts.txt`.
- **Safe Input Handling:** Uses `fgets()` with `strtol()` / `strtod()` so invalid characters never trigger infinite `scanf()` loops.
- **Core Banking Operations:** Check balance, deposit, withdraw (with insufficient-balance protection and high-value notice for withdrawals `>= 25000.00`), mini-statement, and 4-digit PIN change.
- **Persistent Storage:** Rewrites `accounts.txt` after balance/PIN updates and appends timestamped receipts to `transactions.txt`.

## How to Compile & Run

### On Windows / Linux / macOS (GCC)

Make sure `atm_simulator.c` and `accounts.txt` are in the same folder, then run:

```bash
gcc -Wall -Wextra -std=c11 atm_simulator.c -o atm.exe
.\atm.exe
```

*(To share a standalone Windows `.exe` with teammates without MinGW DLL issues, compile with `gcc -static atm_simulator.c -o atm.exe`.)*

### Online (No Compiler Needed)

1. Open OnlineGDB's C Compiler.
2. Paste `atm_simulator.c` and create a second file named `accounts.txt` with the account lines.
3. Click **Run**.

## Sample Test Accounts

| Account No. | Account Holder | PIN | Starting Balance |
| :--- | :--- | :--- | :--- |
| `1001` | Sk Mamun Sohaib | `1234` | `15000.00` |
| `1002` | Subha Patra | `2345` | `22500.00` |
| `1003` | Soham Das | `3456` | `18200.00` |
| `1004` | Prince Biswas | `4567` | `27800.00` |
| `1005` | Riju Samanta | `5678` | `12400.00` |
| `1006` | Sattwik Basu | `6789` | `35000.00` |
| `1007` | Sk Riajul Islam | `7890` | `19600.00` |
| `1008` | Sk Mustak Hassan | `8901` | `24100.00` |
| `1009` | Sk Amir Sohel | `9012` | `31500.00` |
| `1010` | Ayan Malik | `4321` | `32000.00` |
| `1011` | Subhash Kumar | `1122` | `8500.00` |
