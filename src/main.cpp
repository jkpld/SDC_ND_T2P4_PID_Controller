#include <uWS/uWS.h>
#include <iostream>
#include <iomanip>
#include "json.hpp"
#include "PID.h"
#include <math.h>
#include "Twiddle.h"
#include <vector>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  // Car controller
  PID pid;
  // pid.Init(0.07, 0.007, 1); // initial params before twiddle
  pid.Init(0.0585, 0.008, 1.1); // best params by twiddle

  // Twiddle parameters
  Twiddle twiddler;
  bool optimize = false;
  const int N = 1500;
  double ControlError[N] = {};
  std::vector<double> params = {0.07, 0.007, 1};
  std::vector<double> dp = {0.01, 0.001, 0.1};
  twiddler.Init(params,dp,0.01);

  // Speed control
  PID speedControl;
  speedControl.Init(0.5,0,1);
  double setSpeed = 50;

  int counter = 0;

  h.onMessage([&pid,&speedControl,&setSpeed,&twiddler,&ControlError,&counter,optimize](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          // double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;

          // double ctek1 = cte + sin(deg2rad(angle))*speed/100; // assumes straight road
          pid.UpdateError(cte);
          speedControl.UpdateError(speed - setSpeed);
          steer_value = pid.TotalError();

          // DEBUG
          // std::cout << counter << "    "
          //           << std::setw(10) << cte
          //           << std::setw(10) << speed
          //           << std::setw(10) << angle
          //           << std::setw(15) << steer_value
          //           << std::setw(15) << pid.p_error
          //           << std::setw(15) << pid.d_error
          //           << std::setw(15) << pid.i_error
          //           << std::setw(10) << std::endl;

          if (steer_value > 1) steer_value = 1;
          if (steer_value < -1) steer_value = -1;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = speedControl.TotalError();
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";

          if (optimize) {
            if (speed > 0.9*setSpeed) {

              ControlError[counter] = cte*cte;

              if (counter == N-1) {
                double tolErr = 0;
                for (auto ei : ControlError) tolErr += ei;
                tolErr = sqrt(tolErr/N);

                twiddler.UpdateParams(tolErr);
                pid.Kp = twiddler.p[0];
                pid.Ki = twiddler.p[1];
                pid.Kd = twiddler.p[2];

                if (twiddler.finished) {
                  std::cout << "\nFinished! Final values are\n"
                            << "  Kp = " << pid.Kp << std::endl
                            << "  Ki = " << pid.Ki << std::endl
                            << "  Kd = " << pid.Kd << std::endl;
                } else {
                  std::cout << "Iteration : " << twiddler.IterNum << ",\t"
                            << "Best error : " << twiddler.best_err << ",\t"
                            << "Current error : " << tolErr << std::endl;
                  std::cout << "  Kp = " << pid.Kp << "(" << twiddler.dp[0] << "),"
                            << "  Ki = " << pid.Ki << "(" << twiddler.dp[1] << "),"
                            << "  Kd = " << pid.Kd << "(" << twiddler.dp[2] << ")\n";
                }
              }

              counter = (counter + 1) % N;
            }
          }
          // std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
