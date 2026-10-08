"""
Helper functions for ADL to Coffea conversion
"""
import awkward as ak
import numpy as np


def fill_histogram(hist_def, events):
    """
    Fill a histogram with events from a region.
    
    Args:
        hist_def: Dictionary containing histogram definition with 'hist' and 'fill_var'
        events: Awkward array of events to fill
    """
    hist_obj = hist_def['hist']
    fill_var = hist_def['fill_var']
    
    # Evaluate the fill variable
    values = eval(fill_var, {'events': events, 'ak': ak, 'np': np})
    
    # Flatten if needed
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