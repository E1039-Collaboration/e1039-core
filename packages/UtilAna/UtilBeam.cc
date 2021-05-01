#include "UtilBeam.h"
using namespace std;
namespace UtilBeam {

/// Make a list of QIE RF+nn values.
/**
 * The possible values for QIE RF+nn are not evenly distributed.
 * This function just fills `list_values` with a list of all values.
 * The values are taken from "float_2_linrom-2.xls" in DocDB 537.
 */
void ListOfRfValues(int& n_value, int*& list_values)
{
  static int idx = 0;
  static int list[256];
  if (idx == 0) {
    int val = 0;
    list[idx++] = val;
    for (int i = 0; i < 16; i++) { val +=    1; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=    2; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=    4; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=    8; list[idx++] = val; }
    for (int i = 0; i < 15; i++) { val +=   16; list[idx++] = val; }
    for (int i = 0; i <  1; i++) { val +=   31; list[idx++] = val; }
    for (int i = 0; i <  4; i++) { val +=   16; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=   32; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=   64; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=  128; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val +=  256; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val +=  512; list[idx++] = val; }
    for (int i = 0; i < 20; i++) { val += 1024; list[idx++] = val; }
    for (int i = 0; i < 21; i++) { val += 2048; list[idx++] = val; }
    for (int i = 0; i <  6; i++) { val += 4096; list[idx++] = val; }
  }
  n_value = idx;
  list_values = list;
}

/// Make a list of histogram bin edges for the QIE RF+nn value.
/**
 * The possible values for QIE RF+nn are not evenly distributed.
 * This function generates a proper set of histogram bin edges for them.
 *
 * ```
 * int n_edge;
 * double* list_edges;
 * UtilBeam::ListOfRfValueEdges(n_edge, list_edges);
 * TH1* h1_rf_inte = new TH1D("h1_rf_inte", "", n_edge, list_edges);
 * ```
 */
void ListOfRfValueEdges(int& n_edge, double*& list_edges)
{
  static int num = 0;
  static double list[256];
  if (num == 0) {
    int n_value;
    int* list_values;
    ListOfRfValues(n_value, list_values);
    num = n_value + 1;
    
    list[0] = -0.5;
    for (int i = 1; i < n_value; i++) list[i] = (list_values[i] + list_values[i-1]) / 2.0;
    list[n_value] = list_values[n_value-1] + 4096/2;
    
    //cout << "n = " << n_value << "\n";
    //for (int i = 0; i < n_value; i++) cout << i << "\t" << list_values[i] << "\t" << list_edges[i] << "\n";
    //cout << "\t\t" << list_edges[n_value] << endl;
  }
  n_edge = num;
  list_edges = list;
}

}; // End of "namespace UtilBeam"
