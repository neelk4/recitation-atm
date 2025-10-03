#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include "atm.hpp"
#include "catch.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
//                             Helper Definitions //
/////////////////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) {
    return false;  // file problem
  }

  std::string f1_read;
  std::string f2_read;
  while (f1.good() || f2.good()) {
    f1 >> f1_read;
    f2 >> f2_read;
    if (f1_read != f2_read || (f1.good() && !f2.good()) ||
        (!f1.good() && f2.good()))
      return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Test Cases
/////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Example: Create a new account", "[ex-1]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);

  Account sam_account = accounts[{12345678, 1234}];
  REQUIRE(sam_account.owner_name == "Sam Sepiol");
  REQUIRE(sam_account.balance == 300.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);
  std::vector<std::string> empty;
  REQUIRE(transactions[{12345678, 1234}] == empty);
}

TEST_CASE("Example: Simple widthdraw", "[ex-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.WithdrawCash(12345678, 1234, 20);
  auto accounts = atm.GetAccounts();
  Account sam_account = accounts[{12345678, 1234}];

  REQUIRE(sam_account.balance == 280.30);
}

TEST_CASE("Example: Print Prompt Ledger", "[ex-3]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto& transactions = atm.GetTransactions();
  transactions[{12345678, 1234}].push_back(
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $32000.00, Updated Balance: $72099.90");
  atm.PrintLedger("./prompt.txt", 12345678, 1234);
  REQUIRE(CompareFiles("./ex-1.txt", "./prompt.txt"));
}

TEST_CASE("RegisterAccount duplicate throws", "[reg-dup]") {
  Atm atm;
  atm.RegisterAccount(1111, 2222, "Alice", 100.0);
  REQUIRE_THROWS_AS(atm.RegisterAccount(1111, 2222, "AliceAgain", 200.0),
                    std::invalid_argument);
}

TEST_CASE("WithdrawCash negative amount throws", "[wd-neg]") {
  Atm atm;
  atm.RegisterAccount(2222, 3333, "Bob", 200.0);
  REQUIRE_THROWS_AS(atm.WithdrawCash(2222, 3333, -50.0), std::invalid_argument);
}

TEST_CASE("WithdrawCash overdraft throws", "[wd-over]") {
  Atm atm;
  atm.RegisterAccount(3333, 4444, "Charlie", 50.0);
  REQUIRE_THROWS_AS(atm.WithdrawCash(3333, 4444, 100.0), std::runtime_error);
}

TEST_CASE("DepositCash negative amount throws", "[dep-neg]") {
  Atm atm;
  atm.RegisterAccount(4444, 5555, "Dana", 100.0);
  REQUIRE_THROWS_AS(atm.DepositCash(4444, 5555, -10.0), std::invalid_argument);
}

TEST_CASE("DepositCash updates balance and logs", "[dep-ok]") {
  Atm atm;
  atm.RegisterAccount(5555, 6666, "Eve", 10.0);
  atm.DepositCash(5555, 6666, 90.0);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts[{5555, 6666}].balance == 100.0);
  auto& txs = atm.GetTransactions();
  REQUIRE_FALSE(txs[{5555, 6666}].empty());
}

TEST_CASE("WithdrawCash updates balance and logs", "[wd-ok]") {
  Atm atm;
  atm.RegisterAccount(6666, 7777, "Frank", 200.0);
  atm.WithdrawCash(6666, 7777, 50.0);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts[{6666, 7777}].balance == 150.0);
  auto& txs = atm.GetTransactions();
  REQUIRE_FALSE(txs[{6666, 7777}].empty());
}

TEST_CASE("CheckBalance works", "[chk]") {
  Atm atm;
  atm.RegisterAccount(7777, 8888, "Grace", 300.0);
  REQUIRE(atm.CheckBalance(7777, 8888) == Approx(300.0));
}

TEST_CASE("PrintLedger invalid account throws", "[ledger-bad]") {
  Atm atm;
  REQUIRE_THROWS_AS(atm.PrintLedger("bad_ledger.txt", 9999, 1111),
                    std::invalid_argument);
}