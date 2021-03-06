#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <pam/pam.h>
#include <pam/get_time.h>
#include <pam/parse_command_line.h>
#include <parlay/random.h>
#include <parlay/primitives.h>

#include "seg_utils_horizontal.h"
#include "segment2.h"

using namespace std;
timer reserve_tm, build_tm, query_tm;

void reset_timers() {
    reserve_tm.reset(); build_tm.reset(); query_tm.reset();
}

int main(int argc, char** argv) {
  commandLine P(argc, argv,
		"./seg_test [-n size] [-l rmin] [-h rmax] [-r rounds] [-q queries] [-d dist] [-w window] [-t query_type]");
  size_t n = P.getOptionLongValue("-n", 100000000);
  int min_val = P.getOptionIntValue("-l", 0);
  int max_val = P.getOptionIntValue("-h", 1000000000);
  size_t iterations = P.getOptionIntValue("-r", 3);
  dist = P.getOptionIntValue("-d", 0);
  win = P.getOptionIntValue("-w", 1000000);
  size_t query_num = P.getOptionLongValue("-q", 1000);
  int type = P.getOptionIntValue("-t", 1);

  srand(2017);
  if (type == 0) return 0;

    size_t threads = parlay::num_workers();

    parlay::sequence<size_t> r(query_num);	
    parlay::sequence<seg_type_2d> segs = generate_segs(n, min_val, max_val);
    parlay::sequence<query_type> queries = generate_queries(query_num, min_val, max_val);

    for (size_t i = 0; i < iterations; ++i) {
      {
      reserve_tm.start();
      segment_map_2d::reserve(n);
      reserve_tm.stop();
      
      build_tm.start();
      segment_map_2d a = segment_map_2d(segs);
      build_tm.stop();

      print_stats();
	  
      query_tm.start();
      parlay::parallel_for (0, query_num, [&] (size_t i) {
	  r[i] = a.count(queries[i]);
	});
      query_tm.stop();

      size_t total = parlay::reduce(r);

      cout << "RESULT" << fixed << setprecision(3)
         << "\talgo=" << "SegCount"
         << "\tname=" << "Query-Sum"
         << "\tn=" << n
         << "\tq=" << query_num
         << "\tp=" << threads
         << "\tmin-val=" << min_val
         << "\tmax-val=" << max_val
         << "\twin-mean=" << win
         << "\titeration=" << i
         << "\tbuild-time=" << build_tm.get_total()
         << "\treserve-time=" << reserve_tm.get_total()
         << "\tquery-time=" << query_tm.get_total()
         << "\ttotal=" << total
         << endl;


      reset_timers();
      }
      segment_map_2d::finish();
    }
    return 0;
}

