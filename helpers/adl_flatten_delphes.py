#!/usr/bin/env python3

import array
import ROOT

def progress_bar(place, total, steps):

    percent_done = float(place)/float(total)
    steps_done = int(percent_done*steps)

    old_percent_done = float(place-1)/float(total)
    old_steps_done = int(old_percent_done*steps)


    if place == total or place == total-1:
        blank_line = [" " for _ in range(steps+2*len(str(total)) + 10)]
        print("".join(blank_line), end="\r")
        return

    if (old_steps_done == steps_done and place != 0):
        return


    if steps_done == 0:
        steps_done = 1
    elif steps_done == steps:
        steps_done = steps-1
    
    out_chars = ["#" if i <= steps_done else "." for i in range(steps)]
    preamble_chars = [" " for _ in range(len(str(total))-len(str(place)))]

    out_str = "   "  + str(place) + "".join(preamble_chars) + "/" + str(total) + "   [" + "".join(out_chars) + "]\r"

    print(out_str, end="")


TYPE_MAP = {
    "Bool_t": ("i", "I"),
    "Char_t": ("i", "I"),
    "UChar_t": ("i", "I"),
    "Short_t": ("i", "I"),
    "UShort_t": ("i", "I"),
    "Int_t": ("i", "I"),
    "UInt_t": ("I", "i"),
    "Long_t": ("l", "L"),
    "ULong_t": ("L", "l"),
    "Long64_t": ("q", "L"),
    "ULong64_t": ("Q", "l"),
    "Float_t": ("f", "F"),
    "Double_t": ("d", "D"),
    "bool": ("i", "I"),
    "char": ("i", "I"),
    "unsigned char": ("i", "I"),
    "short": ("i", "I"),
    "unsigned short": ("i", "I"),
    "int": ("i", "I"),
    "unsigned int": ("I", "i"),
    "long": ("l", "L"),
    "unsigned long": ("L", "l"),
    "float": ("f", "F"),
    "double": ("d", "D"),
}


def _discover_delphes_collections(chain):
    """
    Discover top-level Delphes TClonesArray branches automatically.

    In Delphes files, collections are usually top-level branches such as:
      Event, Electron, Muon, Jet, Photon, MissingET, ScalarHT, GenParticle, ...

    This function avoids subbranches like:
      Electron.PT
      Electron.Eta
      Jet.Mass
    """

    collections = []

    branches = chain.GetListOfBranches()

    for br in branches:
        name = br.GetName()

        if "." in name:
            continue

        if name.endswith("_size"):
            continue

        class_name = ""
        try:
            class_name = br.GetClassName()
        except Exception:
            class_name = ""

        # Most Delphes collections are TClonesArray branches.
        # Keep TClonesArray, and also keep branches whose names look like
        # Delphes top-level collections.
        if class_name == "TClonesArray":
            collections.append(name)
            continue

        # Some ROOT/Delphes configurations report less information.
        # Test if ExRootTreeReader can use the branch later.
        # Here we still keep plausible top-level branches.
        if name and not name.startswith("f"):
            collections.append(name)

    # Remove duplicates while preserving order
    out = []
    seen = set()
    for c in collections:
        if c not in seen:
            out.append(c)
            seen.add(c)

    return out


def _discover_members_from_object(obj):
    if not obj:
        return []

    try:
        tclass = obj.IsA()
    except Exception:
        return []

    return _discover_members_from_class(tclass)


def _discover_members_from_class(tclass):
    """
    Discover scalar numeric public data members using ROOT reflection.

    Returns list of:
      (member_name, cpp_type, python_array_type, root_leaf_type)
    """

    if not tclass:
        return []

    members = []
    seen = set()

    classes_to_scan = [tclass]

    bases = tclass.GetListOfBases()
    if bases:
        for base in bases:
            base_name = base.GetName()
            base_class = ROOT.TClass.GetClass(base_name)
            if base_class:
                classes_to_scan.append(base_class)

    for cls in classes_to_scan:
        data_members = cls.GetListOfDataMembers()
        if not data_members:
            continue

        for dm in data_members:
            name = dm.GetName()

            if name in seen:
                continue

            seen.add(name)

            if not name:
                continue

            if "[" in name:
                continue

            try:
                if dm.GetArrayDim() > 0:
                    continue
            except Exception:
                pass

            cpp_type = dm.GetTypeName()

            if not cpp_type:
                continue

            if "*" in cpp_type:
                continue

            cpp_type = cpp_type.strip()

            if cpp_type not in TYPE_MAP:
                continue

            py_type, root_type = TYPE_MAP[cpp_type]
            members.append((name, cpp_type, py_type, root_type))

    return members


def _find_first_object(reader, branch, n_entries):
    """
    Find first available object in a Delphes branch so that its class can be inspected.
    """

    for i in range(n_entries):
        reader.ReadEntry(i)
        if branch.GetEntries() > 0:
            return branch.At(0)

    return None


def _build_event_tree(collections):
    tree = ROOT.TTree("Events", "Event-level collection multiplicities")

    buffers = {}

    event = array.array("i", [0])
    _make_branch(tree, "event", event, "I")
    buffers["event"] = event

    for coll in collections:
        arr = array.array("i", [0])
        name = f"n_{_sanitize_name(coll)}"
        _make_branch(tree, name, arr, "I")
        buffers[f"n_{coll}"] = arr

    return tree, buffers


def _build_collection_tree(collection_name, members):
    """
    Create one flat tree per collection.
    """

    tree_name = _sanitize_name(collection_name)
    tree = ROOT.TTree(tree_name, f"Flat Delphes collection: {collection_name}")

    buffers = {}

    event = array.array("i", [0])
    index = array.array("i", [0])

    _make_branch(tree, "event", event, "I")
    _make_branch(tree, f"{tree_name.lower()}_index", index, "I")

    buffers["event"] = event
    buffers["index"] = index

    used_branch_names = set(["event", f"{tree_name.lower()}_index"])

    for member_name, cpp_type, py_type, root_type in members:
        branch_name = _sanitize_name(member_name)

        if branch_name in used_branch_names:
            branch_name = f"{tree_name.lower()}_{branch_name}"

        used_branch_names.add(branch_name)

        arr = array.array(py_type, [_default_value(py_type)])
        _make_branch(tree, branch_name, arr, root_type)

        buffers[member_name] = arr

    return tree, buffers


def _make_branch(tree, name, arr, root_type):
    tree.Branch(name, arr, f"{name}/{root_type}")


def _sanitize_name(name):
    return (
        str(name)
        .replace(".", "_")
        .replace(" ", "_")
        .replace("[", "_")
        .replace("]", "")
        .replace("-", "_")
        .replace("/", "_")
    )


def _default_value(py_type):
    if py_type in ("f", "d"):
        return -999.0
    return -999


def _safe_get(obj, attr, default):
    try:
        return getattr(obj, attr)
    except Exception:
        return default


def flatten_delphes(
    input_files,
    output_file="flat_delphes.root",
    tree_name="Delphes",
    verbose=True,
):
    """
    Automatically flatten all Delphes collections in a ROOT TTree.

    Parameters
    ----------
    input_files : str or list[str]
        One input ROOT file or a list of ROOT files.
    output_file : str
        Output flat ROOT file.
    tree_name : str
        Delphes TTree name, usually "Delphes".
    delphes_lib : str or None
        Delphes library to load. Usually "libDelphes" or "/path/to/libDelphes.so".
        Set to None if already loaded.
    verbose : bool
        Print progress and discovered collections.

    Returns
    -------
    str
        Path to output ROOT file.
    """

    ROOT.gROOT.SetBatch(True)

    if isinstance(input_files, str):
        input_files = [input_files]

    load_status = ROOT.gSystem.Load("libDelphes")
    if load_status < 0:
        print("Did not load correctly")

    ROOT.gInterpreter.Declare('#include "classes/DelphesClasses.h"')
    ROOT.gInterpreter.Declare('#include "external/ExRootAnalysis/ExRootTreeReader.h"')

    chain = ROOT.TChain(tree_name)
    for fname in input_files:
        chain.Add(fname)

    total_entries = chain.GetEntries()
    if total_entries <= 0:
        raise RuntimeError(
            f"No entries found in tree '{tree_name}'. Check input files."
        )

    reader = ROOT.ExRootTreeReader(chain)

    n_entries = reader.GetEntries()

    collections = _discover_delphes_collections(chain)

    input_branches = {}
    for coll in collections:
        input_branches[coll] = reader.UseBranch(coll)

    collection_members = {}

    for coll in collections:
        obj = _find_first_object(reader, input_branches[coll], n_entries)
        members = _discover_members_from_object(obj)
        collection_members[coll] = members

        types_map = {}
        for name, cpp_type, _, _ in members:
            if cpp_type not in types_map.keys():
                types_map[cpp_type] = []
            types_map[cpp_type].append(name)

        for type in types_map.keys():
            print(f"{type} : {types_map[type]}")

    out_file = ROOT.TFile(output_file, "RECREATE")

    event_tree, event_buffers = _build_event_tree(collections)

    collection_trees = {}
    collection_buffers = {}

    for coll in collections:
        tree, buffers = _build_collection_tree(coll, collection_members[coll])
        collection_trees[coll] = tree
        collection_buffers[coll] = buffers

    for i_entry in range(n_entries):
        
        progress_bar(i_entry, n_entries, 100)

        reader.ReadEntry(i_entry)

        event_buffers["event"][0] = i_entry

        for coll in collections:
            event_buffers[f"n_{coll}"][0] = input_branches[coll].GetEntries()

        event_tree.Fill()

        for coll in collections:
            branch = input_branches[coll]
            tree = collection_trees[coll]
            buffers = collection_buffers[coll]
            members = collection_members[coll]

            n_obj = branch.GetEntries()

            for i_obj in range(n_obj):
                obj = branch.At(i_obj)

                buffers["event"][0] = i_entry
                buffers["index"][0] = i_obj

                for member_name, cpp_type, py_type, root_type in members:
                    default = _default_value(py_type)
                    value = _safe_get(obj, member_name, default)

                    if py_type in ("i", "I", "l", "L", "q", "Q"):
                        try:
                            value = int(value)
                        except Exception:
                            value = int(default)
                    else:
                        try:
                            value = float(value)
                        except Exception:
                            value = float(default)

                    buffers[member_name][0] = value

                tree.Fill()

    out_file.cd()
    event_tree.Write()

    for coll in collections:
        collection_trees[coll].Write()

    out_file.Close()

    if verbose:
        print(f"\n[info] Wrote {output_file}")

    return output_file
