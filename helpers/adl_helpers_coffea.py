"""
Helper functions for ADL to Coffea conversion
"""
import awkward as ak
import numpy as np


def fill_histogram(hist_def, events):
    hist_obj = hist_def['hist']

    if 'fill_expr_x' in hist_def and 'fill_expr_y' in hist_def:
        x_values = hist_def['fill_expr_x'](events)
        y_values = hist_def['fill_expr_y'](events)

        if hasattr(x_values, 'ndim') and x_values.ndim > 1:
            x_values = ak.flatten(x_values)
        if hasattr(y_values, 'ndim') and y_values.ndim > 1:
            y_values = ak.flatten(y_values)

        hist_obj.fill(x=ak.to_numpy(x_values), y=ak.to_numpy(y_values))
        return

    if 'fill_expr' in hist_def:
        values = hist_def['fill_expr'](events)
    else:
        fill_var = hist_def['fill_var']
        values = eval(fill_var, {'events': events, 'ak': ak, 'np': np})

    if hasattr(values, 'ndim') and values.ndim > 1:
        values = ak.flatten(values)

    hist_obj.fill(x=ak.to_numpy(values))

def fill_histogram_list(hist_list, events):
    """
    Fill a list of histograms with events from a region.
    
    Args:
        hist_list: List of histogram definitions
        events: Awkward array of events to fill
    """
    for hist_def in hist_list:
        fill_histogram(hist_def, events)


def create_function_from_table(table):
    """
    Create a lookup function from a table of values and bounds.
    
    Args:
        table: List of tuples (value, lower_bounds, upper_bounds)
    
    Returns:
        A function that takes arguments and returns the appropriate value
    """
    def lookup(*args):
        for entry in table:
            value, lower_bounds, upper_bounds = entry
            in_bounds = True
            for i, arg in enumerate(args):
                if not (lower_bounds[i] <= arg <= upper_bounds[i]):
                    in_bounds = False
                    break
            if in_bounds:
                if isinstance(value, tuple):
                    return value[0]  # Return central value
                return value
        return 0  # Default if no match
    
    return lookup


def combine_without_duplicates(list1, list2):
    """
    Combine two lists while removing duplicates.
    """
    combined = list(list1)
    for item in list2:
        if item not in combined:
            combined.append(item)
    return combined


def try_create_provenance(events_collection, pt_field="pt"):
    """
    Attempt to create provenance indices for a collection.
    Returns ak.local_index array if the collection is jagged and has the
    requested pt field, otherwise returns None.
    """
    import awkward as ak
    
    try:
        field_data = getattr(events_collection, pt_field)
    except AttributeError:
        return None
    
    # Only create provenance for jagged (variable-length list) arrays
    # ndim >= 2 means at least one list dimension (events × objects)
    if field_data.ndim < 2:
        return None
    
    return ak.local_index(field_data, axis=1)

def concat_provenance(prov_a, prov_b):
    import awkward as ak
    
    if prov_a is None or prov_b is None:
        return None
    
    return ak.concatenate([prov_a, prov_b], axis=1)

def check_distinct(prov0, prov1, pt0, pt1, eta0, eta1, phi0, phi1):
    """
    Check distinctness of two particle arrays.
    If both provenance arrays are available (not None), use index comparison.
    Otherwise fall back to kinematic comparison.
    """
    import awkward as ak
    
    if prov0 is not None and prov1 is not None:
        return (prov0 != prov1)
    else:
        return (pt0 != pt1) | (eta0 != eta1) | (phi0 != phi1)

def delta_r_cross(col_a, col_b):
    """
    Compute delta_r between all pairs from two collections.
    Returns a doubly-jagged array: [event][pair_from_a × pair_from_b].
    """
    a, b = ak.unzip(ak.cartesian([col_a, col_b], axis=1, nested=True))
    return a.delta_r(b)


def delta_phi_cross(col_a, col_b):
    """
    Compute delta_phi between all pairs from two collections.
    Returns a doubly-jagged array: [event][pair_from_a × pair_from_b].
    """
    a, b = ak.unzip(ak.cartesian([col_a, col_b], axis=1, nested=True))
    return abs(a.delta_phi(b))

def cutflow_generate(reg_name, steps, file_out=None):
    """Print cutflow table in LaTeX format matching TIMBER output.
    
    Args:
        reg_name: clean region name for display
        steps: OrderedDict of {description: count} including 'Initial' as first entry
        file_out: base filename (without .txt) to append to, or None for stdout
    """
    import re, sys
    orig_stdout = sys.stdout
    file = open(file_out + '.txt', 'a') if file_out is not None else None

    try:
        if file is not None:
            sys.stdout = file

        clean = re.sub(r'[A-Za-z0-9]*REG', '', reg_name)
        clean = re.sub(r'[A-Za-z0-9]*UNION', '', clean)

        print('\n---\n \\begin{tabular}{c c c c} \\multicolumn{4}{c}{Cutflow report for region '
              + clean
              + '}\\\\ \\hline Cut & Events left & Eff from previous & Eff from initial \\\\ \\hline')

        _init = 0
        _prev = 0

        for _k, _v in steps.items():
            _name = re.sub(r'[A-Za-z0-9]*UNION', '', _k)
            if _k == 'Initial':
                _init = _v
                _prev = _init
            print('\\verb`' + _name + '` & ' + str(int(_v)) + ' & '
                  + f'{(_v / (_prev + 1e-9)):.2%}'[:-1] + '\\% & '
                  + f'{(_v / (_init + 1e-9)):.4%}'[:-1] + '\\%\\\\')
            _prev = _v

        print('\\end{tabular} \n---\n')
    finally:
        sys.stdout = orig_stdout
        if file is not None:
            file.close()


def print_all_cutflows(output, file_out=None):
    """Find all cutflow entries in output and print reports.
    
    Expects flat keys like:
        cutflow_<region>::Initial
        cutflow_<region>::description1
        cutflow_<region>::description2
    """
    from collections import OrderedDict
    regions = OrderedDict()
    for key in output.keys():
        if key.startswith("cutflow_") and "::" in key:
            rest = key[len("cutflow_"):]
            reg_name, step_desc = rest.split("::", 1)
            if reg_name not in regions:
                regions[reg_name] = OrderedDict()
            regions[reg_name][step_desc] = output[key]
    for reg_name, steps in regions.items():
        cutflow_generate(reg_name, steps, file_out=file_out)

def combine_outputs(a, b):
    result = dict(a)
    for k, v in b.items():
        if isinstance(v, (int, float, np.number)):
            result[k] = result.get(k, 0.0) + v
        elif k.startswith("cutflow_") and isinstance(v, dict):
            existing = result.get(k, {})
            combined = dict(existing)
            for ck, cv in v.items():
                combined[ck] = combined.get(ck, 0.0) + cv
            result[k] = combined
        else:
            result[k] = v
    return result

def ToLV(pt, eta, phi, mass):
    import coffea
    return ak.zip({'pt': pt,'eta': eta, 'phi': phi, 'mass': mass}, with_name='PtEtaPhiMLorentzVector', behavior=coffea.nanoevents.methods.vector.behavior)