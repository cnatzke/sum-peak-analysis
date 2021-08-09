#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <chrono>
#include <iostream>
#include <iomanip>

class ProgressBar {
   private:
      unsigned int ticks = 0;

      const unsigned int total_ticks;
      const unsigned int bar_width;
      const char complete_char = '=';
      const char incomplete_char = ' ';
      const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

   public:
      ProgressBar(unsigned int total, unsigned int width, char complete, char incomplete) :
         total_ticks {total}, bar_width {width}, complete_char {complete}, incomplete_char {incomplete} {}

      ProgressBar(unsigned int total, unsigned int width) : total_ticks {total}, bar_width {width} {}

      unsigned int operator++() { return ++ticks; }

      void display() const
      {
         float progress = (float) ticks / total_ticks;
         unsigned int pos = (unsigned int) (bar_width * progress);

         std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
         auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time).count(); //ms

         // time estimate
         auto time_remaining = (float(time_elapsed) / ticks) * (total_ticks - ticks); // ms

         std::cout << "Event " << ticks << "/" << total_ticks << " [";

         for (unsigned int i = 0; i < bar_width; ++i) {
            if (i < pos) std::cout << complete_char;
            else if (i == pos) std::cout << ">";
            else std::cout << incomplete_char;
         }
         std::cout << "] " << int(progress * 100.0) << "%"
            << " | " << std::setprecision(2) << std::fixed << float(time_remaining) / 1000 << "s remaining"
            << " | " << float(time_elapsed) / 1000.0 << "s elapsed \r";
         std::cout.flush();
      }

      void done() const
      {
         display();
         std::cout << std::endl;

         /*
         // optional formatting end complete progress bar
         std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
         auto time_total = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
         std::cout << "[" << std::string(bar_width, complete_char)
         << "] 100% "
         << float(time_total) / 1000. << "s Elapsed"
         << std::string(3, ' ') << std::endl;
         */
      }
};

#endif //PROGRESSBAR_H
