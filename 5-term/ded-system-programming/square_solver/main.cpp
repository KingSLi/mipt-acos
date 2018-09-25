#include <iostream>
#include <cassert>
#include <cmath>

/*!
 * Uncomment TESTING if you want run tests
 */
// #define TESTING

/*!
 * INF_ROOTS is value means that equation have infinity count of roots
 */
 const int INF_ROOTS = INT32_MAX;

/*!
 * PRECISION is accuracy of compare double
 */
 const double PRECISION = 1e-8;

/*!
 * Check is a equal zero with PRECISION
 * @param a
 * @return bool if a == 0, else false
 */
inline bool isZero(double a) {
    return (fabs(a) < PRECISION);
}

/*!
 * This function solve linear equation b*x + c = 0
 * solve - is place for root of equation
 * @return count of roots
 */
int solve_linear(double b, double c, double& root);

/*!
 * This function solve square equation
 * a, b, c is (double) coef of equation a*x^2 + b*x + c = 0
 * solve1, solve2 is place for solves of equation
 * @return (int) count of roots
 */
int solve_square(double a, double b, double c, double &solve1, double &solve2);

#ifndef TESTING

int main() {
    printf("Hello! Please enter coef of equation in format: a b c\n");
    double a = NAN;
    double b = NAN;
    double c = NAN;
    int count_input = scanf("%lf %lf %lf", &a, &b, &c);
    if (count_input != 3) {
        printf("ERROR: wrong input data");
        return 0;
    }
    assert(a != NAN);
    assert(b != NAN);
    assert(c != NAN);

    printf("You want to solve equation: %lf*x^2 + (%lf)*x + (%lf) = 0\n", a, b, c);

    double root1 = NAN, root2 = NAN;
    int countRoots = solve_square(a, b, c, root1, root2);

    switch (countRoots) {
        case 0:
            // ...doesn't have roots + \n plz
            printf("This equation haven't roots");
            break;
        case 1:
            assert(root1 != NAN);
            printf("This equation have one root x = %lf", root1);
            break;
        case 2:
            assert(root1 != NAN);
            assert(root2 != NAN);
            printf("This equation have two roots x1 = %lf, x2 = %lf", root1, root2);
            break;
        case INF_ROOTS:
            printf("This equation have INFINITY roots");
            break;
        default:
            printf("ERROR: error in solve equation");
    }
    return 0;
}




#else

#include <vector>

void TEST(double a, double b, double c, int count_answer, double ans1, double ans2) {
    double root1 = NAN;
    double root2 = NAN;
    int count = solve_square(a, b, c, root1, root2);
    bool has_error = false;
    if (count != count_answer)
        has_error = true;

    if (has_error) {
        printf("TEST FAILED ############\n"
               "input: a: %lf\t b: %lf\t c: %lf\n"
               "program output: count roots:%d\t %lf %lf\n"
               "right output: count roots:%d\t %lf %lf\n"
               "########################\n", a, b, c, count, root1, root2, count_answer, ans1, ans2);
    }
}

struct Tester{
    double a, b, c;
    int count_roots;
    double ans1, ans2;
    Tester(double a_, double b_, double c_, int count_, double ans1_, double ans2_) :
            a(a_),
            b(b_),
            c(c_),
            count_roots(count_),
            ans1(ans1_),
            ans2(ans2_) {}
};

int main() {
    std::vector<Tester> test_data;
    test_data.emplace_back(Tester(0, 1, 1, 1, -1, NAN));
    test_data.emplace_back(Tester(1, 0, 1, 0, NAN, NAN));
    test_data.emplace_back(Tester(0, 0, 0, INF_ROOTS, NAN, NAN));
    test_data.emplace_back(Tester(1, -2, 1, 1, 1, NAN));

    for (auto test : test_data) {
        TEST(test.a, test.b, test.c, test.count_roots, test.ans1, test.ans2);
    }
}
#endif


int solve_linear(double b, double c, double& root) {
    assert(std::isfinite(b));
    assert(std::isfinite(c));
    assert(&root != nullptr);

    if (isZero(b)) {
        if (isZero(c))
            return INF_ROOTS;
        root = 0;
        return 1;
    }
    root = -c / b;
    return 1;
}

int solve_square(double a, double b, double c, double &solve1, double &solve2) {

    assert(std::isfinite(a));
    assert(std::isfinite(b));
    assert(std::isfinite(c));

    assert(&solve1 != nullptr);
    assert(&solve2 != nullptr);
    assert(&solve2 != &solve1);

    if (isZero(a))
        return solve_linear(b, c, solve1);

    if (isZero(c)) {
        solve1 = 0;
        int count = solve_linear(a, b, solve2);
        if (count == INF_ROOTS)
            return INF_ROOTS;
        if (isZero(solve1 - solve2))
            return 1;
        return 2;
    }

    double disc = b * b - 4 * a * c;
    if (isZero(disc)) {
        solve1 = -b / (2 * a);
        return 1;
    } else if (disc < 0) {
        return 0;
    }
    double d = sqrt(disc);
    solve1 = (-b + d) / (2 * a);
    solve2 = (-b - d) / (2 * a);
    return 2;

}
