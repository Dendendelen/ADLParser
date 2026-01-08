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
