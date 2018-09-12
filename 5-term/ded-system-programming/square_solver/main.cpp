#include <iostream>
#include <cassert>
#include <cmath>

/*!
 * Uncomment TESTING if you want run tests
 */
// #define TESTING

#define INF_ROOTS -2

/*!
 * This function solve linear equation b*x + c = 0
 * solve - is place for solve of equation
 * @return count of solves
 */
int solve_linear(double b, double c, double& solve) {
  assert(std::isfinite(b));
  assert(std::isfinite(c));
  assert(&solve != nullptr);

  if (b == 0) {
    if (c == 0) {
      return INF_ROOTS;
    } else {
      solve = 0;
      return 1;
    }
  } else {
    solve = -c / b;
    return 1;
  }
}

/*!
 * This function solve square equation
 * a, b, c is (double) coef of equation a*x^2 + b*x + c = 0
 * solve1, solve2 is place for solves of equation
 * @return (int) count of solves
 */
int solve_square(double a, double b, double c, double &solve1, double &solve2) {

  assert(std::isfinite(a));
  assert(std::isfinite(b));
  assert(std::isfinite(c));

  assert(&solve1 != nullptr);
  assert(&solve2 != nullptr);
  assert(&solve2 != &solve1);

  if (a == 0)
    return solve_linear(b, c, solve1);

  double disc = b * b - 4 * a * c;
  if (disc == 0) {
    solve1 = -b / (2 * a);
    return 1;
  } else if (disc < 0) {
    return 0;
  } else {
    solve1 = (-b + sqrt(disc)) / (2 * a);
    solve2 = (-b - sqrt(disc)) / (2 * a);
    return 2;
  }
}

#ifndef TESTING

int main() {
  printf("Hello! Please enter coef of eqution in format: a b c\n");
  double a, b, c, solve1, solve2;
  int count_input = scanf("%lf %lf %lf", &a, &b, &c);
  if (count_input != 3) {
    printf("ERROR: wrong input data");
    return 0;
  }
  printf("You want to solve equation: %lf*x^2 + (%lf)*x + (%lf) = 0\n", a, b, c);
  int countRoots = solve_square(a, b, c, solve1, solve2);
  switch (countRoots) {
    case 0:
      printf("This equation haven't roots");
      break;
    case 1:
      printf("This equation have one root x = %lf", solve1);
      break;
    case 2:
      printf("This equation have two roots x1 = %lf, x2 = %lf", solve1, solve2);
      break;
    case INF_ROOTS:
      printf("This equation have INFINITY roots");
      break;
    default:
      printf("ERROR: error in solve equation");
  }
  printf("\nSolver square equation closed!\n");
  return 0;
}

#else

int main() {
  double solve1, solve2, a, b, c;
  // one root
  a = 0; b = 1; c = 1;
  solve_square(a, b, c, solve1, solve2);
  assert(solve1 == -1);
  // zero roots
  a = 1; b = 0; c = 1;
  assert(solve_square(a, b, c, solve1, solve2) == 0);
  // inf roots
  a = 0; b = 0; c = 0;
  assert(solve_square(a, b, c, solve1, solve2) == INF_ROOTS);
  // repeated roots
  a = 1; b = -2; c = 1; solve2 = -INT32_MAX;
  solve_square(a, b, c, solve1, solve2);
  assert((solve1 == 1) && (solve2 == -INT32_MAX));
  // two roots
  a = 1; b = -5; c = -6;
  solve_square(a, b, c, solve1, solve2);
  assert(((solve1 == -1) && (solve2 == 6)) || ((solve1 == 6) && (solve2 == -1)));
}
#endif

