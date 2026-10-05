#include <iostream>
#include <cmath>
using namespace std;

double f(double x) {
    return x*x*x - x - 2;
}

int main() {
    double a = 1, b = 2, x;
    int k = 0;

    while (fabs(b - a) > 0.0001) {
        x = b - f(b) * (b - a) / (f(b) - f(a));
        if (f(a) * f(x) < 0)
            b = x;
        else
            a = x;
        k++;
    }

    cout << "Түбір жуық мәні: x = " << x << endl;
    cout << "Қадам саны: " << k << endl;
    return 0;
}

