int fib(int n)
{
    if (n < 2)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char *argv)
{
    fib(10);
    while (1);
    return 0;
}
