#include "PID.h"

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp_in, double Ki_in, double Kd_in) {
  Kp = Kp_in;
  Ki = Ki_in;
  Kd = Kd_in;
  p_error = 0;
  i_error = 0;
  d_error = 0;
  is_init_ = false;
}

void PID::UpdateError(double cte) {
  if (!is_init_) {
    p_error = cte;
    is_init_ = true;
  }

  d_error = cte - p_error;
  p_error = cte;
  i_error += cte;
}

double PID::TotalError() {
  return -Kp*p_error -Kd*d_error -Ki*i_error;
}
