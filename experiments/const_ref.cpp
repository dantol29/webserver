class Account
{
  private:
	double balance;

  public:
	Account(double initialBalance) : balance(initialBalance)
	{
	}

	// Getter that returns a const reference to the balance
	const double &getBalance() const
	{
		return balance;
	}
};

int main()
{
	Account myAccount(100.0);

	// Trying to bind a non-const reference to a const reference returned by getBalance()
	double &balanceRef = myAccount.getBalance(); // This line should cause a compile-time error

	// Modify the balance through the non-const reference (if it were allowed)
	balanceRef += 50.0; // Intended to increase balance to 150.0

	return 0;
}
