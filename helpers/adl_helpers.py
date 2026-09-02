def combine_without_duplicates(list1, list2):
    list1_set = set(list1)
    list2_set = set(list2)
    unique_to_second = list2_set - list1_set

    return list1 + list(unique_to_second)

def use_histo(histo_params, node):

    node = node.Clone()

    if len(histo_params) == 6:
        node2 = node.Define('_variable_1', histo_params[5])
        hist = node2.DataFrame.Histo1D((histo_params[0], histo_params[1], histo_params[2], histo_params[3], histo_params[4]), '_variable_1')
    else:
        node2 = node.Define('_variable_1', histo_params[5])
        node3 = node2.Define('_variable_2', histo_params[9])
        hist = node3.DataFrame.Histo2D((histo_params[0], histo_params[1], histo_params[2], histo_params[3], histo_params[4], histo_params[6], histo_params[7], histo_params[8]), '_variable_1', '_variable_2')
    hist.Write()
    print("Created histogram "+ histo_params[0])

def use_histo_list(histo_list, node):
    for histo in histo_list:
        use_histo(histo, node)


def create_function_out_of_table(name, table):

    num_entries = len(table)
    num_bounds_per_entry = len(table[0][1])
    num_vals_per_entry = len(table[0][0])

    lower_bounds_list = np.empty((num_entries, num_bounds_per_entry))
    upper_bounds_list = np.empty((num_entries, num_bounds_per_entry))
    values_list = np.empty((num_entries, num_vals_per_entry))

    for entry, i in zip(table, range(num_entries)):
        value = entry[0]
        lower_bounds = entry[1]
        upper_bounds = entry[2]

        for val, j in zip(value, range(num_vals_per_entry)):
            values_list[i,j] = val
        for lower, j in zip(lower_bounds, range(num_bounds_per_entry)):
            lower_bounds_list[i,j] = lower
        for upper, j in zip(upper_bounds, range(num_bounds_per_entry)):
            upper_bounds_list = upper

    lower_bounds_array = ROOT.VecOps.AsRVec(lower_bounds_list)
    upper_bounds_array = ROOT.VecOps.AsRVec(upper_bounds_list)
    values_array = ROOT.VecOps.AsRVec(values_list)

    ROOT.gInterpreter.Declare(f"""
        auto& lower_bounds_rvec = *reinterpret_cast<ROOT::RVec<ROOT::RVec<double>>*>({ROOT.AddressOf(lower_bounds_array)[0]});
        auto& upper_bounds_rvec = *reinterpret_cast<ROOT::RVec<ROOT::RVec<double>>*>({ROOT.AddressOf(upper_bounds_array)[0]});
        auto& values_rvec = *reinterpret_cast<ROOT::RVec<ROOT::RVec<double>>*>({ROOT.AddressOf(values_array)[0]});
    """)

    ROOT.gInterpreter.Declare(name + ' = create_table_function('+num_bounds_per_entry + ', lower_bounds_rvec, upper_bounds_rvec, values_rvec);')
