#ifndef TWIDDLE_H_
#define TWIDDLE_H_

#include <vector>

class Twiddle {
public:
  std::vector<double> p;
  std::vector<double> dp;
  double tol;
  bool finished;
  double best_err;
  unsigned int IterNum;

  void Init(std::vector<double> p_in, std::vector<double> dp_in, double tol_in);
  void UpdateParams(double err);

private:
  int n;

  bool is_init_;
  unsigned int state[2] = {0,0};
};


void Twiddle::Init(std::vector<double> p_in, std::vector<double> dp_in, double tol_in) {
  n = p_in.size();
  p.resize(n);
  dp.resize(n);

  p = p_in;
  dp = dp_in;
  tol = tol_in;
  IterNum = 0;
  is_init_ = false;
  finished = false;
}

void Twiddle::UpdateParams(double err) {
  if (!is_init_) {
    best_err = err;
    is_init_ = true;
  }

  if (!finished) {
    double sumdp = 0;
    for (auto dpi : dp) sumdp += dpi;
    if (sumdp <= tol) {
      finished = true;
      return;
    }
  }

  switch (state[1]) {
    case 0: {
      // Try increasing the param value and retesting
      p[state[0]] += dp[state[0]];
      // state 1
      state[1] = 1;
      break;
    }
    case 1: {
      // we tried increasing a param value, now see if it worked
      if (err < best_err) {
        best_err = err;
        dp[state[0]] *= 1.3;

        state[1] = 0; //back to state 0
        state[0] = (state[0] + 1) % n; // move to next parameter
        if (state[0]==0) ++IterNum;

      } else {
        p[state[0]] -= 2* dp[state[0]];
        state[1] = 2;
      }
      break;
    }
    case 2: {
      // we tried decreasing the value, see if it worked
      if (err < best_err) {
        best_err = err;
        dp[state[0]] *= 1.3;
      } else {
        p[state[0]] += dp[state[0]];
        dp[state[0]] *= 0.5;
      }

      state[1] = 0; //back to state 0
      state[0] = (state[0] + 1) % n; // move to next parameter
      if (state[0]==0) ++IterNum;

      break;
    }
  }
}

#endif
