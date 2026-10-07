import ROOT


TYPE_MAP = {
    # floats
    "Float_t": ("float", "float", "Delphes_Get_float"),
    "float": ("float", "float", "Delphes_Get_float"),
    "Double_t": ("double", "double", "Delphes_Get_double"),
    "double": ("double", "double", "Delphes_Get_double"),

    # signed ints
    "Char_t": ("int", "int", "Delphes_Get_int"),
    "UChar_t": ("uint", "unsigned int", "Delphes_Get_uint"),
    "Short_t": ("int", "int", "Delphes_Get_int"),
    "UShort_t": ("uint", "unsigned int", "Delphes_Get_uint"),
    "Int_t": ("int", "int", "Delphes_Get_int"),
    "int": ("int", "int", "Delphes_Get_int"),
    "UInt_t": ("uint", "unsigned int", "Delphes_Get_uint"),
    "unsigned int": ("uint", "unsigned int", "Delphes_Get_uint"),

    # long types
    "Long64_t": ("long64", "long long", "Delphes_Get_long64"),
    "long long": ("long64", "long long", "Delphes_Get_long64"),
    "ULong64_t": ("ulong64", "unsigned long long", "Delphes_Get_ulong64"),
    "unsigned long long": ("ulong64", "unsigned long long", "Delphes_Get_ulong64"),

    # bool: store as int RVec
    "Bool_t": ("int", "int", "Delphes_Get_int"),
    "bool": ("int", "int", "Delphes_Get_int"),
}


def _load_delphes(
    load_lib=True,
    declare_delphes_classes=True,
    declare_exroot=True,
):
    if load_lib:
        ROOT.gSystem.Load("libDelphes")

    if declare_delphes_classes:
        ROOT.gInterpreter.Declare('#include "classes/DelphesClasses.h"')

    if declare_exroot:
        ROOT.gInterpreter.Declare('#include "external/ExRootAnalysis/ExRootTreeReader.h"')


_DECLARED_DELPHES_RDF_HELPERS = False


def _declare_rdf_helpers():
    global _DECLARED_DELPHES_RDF_HELPERS

    if _DECLARED_DELPHES_RDF_HELPERS:
        return
    ROOT.gInterpreter.Declare(r"""
#include <TClonesArray.h>
#include <TObject.h>
#include <TClass.h>
#include <TDataMember.h>
#include <TList.h>
#include <ROOT/RVec.hxx>

#include <string>
#include <map>
#include <utility>

using ROOT::VecOps::RVec;

namespace DelphesRDFHelper {

inline int N(const TClonesArray& arr) {
    return arr.GetEntriesFast();
}

inline Long_t GetOffsetByClassName(const std::string& class_name, const std::string& member) {
    static std::map<std::pair<std::string, std::string>, Long_t> cache;

    auto key = std::make_pair(class_name, member);
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }

    TClass* cls = TClass::GetClass(class_name.c_str());
    if (!cls) {
        cache[key] = -1;
        return -1;
    }

    TList* members = cls->GetListOfDataMembers();
    if (!members) {
        cache[key] = -1;
        return -1;
    }

    TDataMember* dm = dynamic_cast<TDataMember*>(members->FindObject(member.c_str()));
    if (!dm) {
        cache[key] = -1;
        return -1;
    }

    Long_t offset = dm->GetOffset();
    cache[key] = offset;
    return offset;
}

template <typename T>
RVec<T> GetByOffset(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member,
    T default_value
) {
    const int n = arr.GetEntriesFast();

    RVec<T> out;
    out.reserve(n);

    const Long_t offset = GetOffsetByClassName(class_name, member);

    if (offset < 0) {
        for (int i = 0; i < n; ++i) {
            out.emplace_back(default_value);
        }
        return out;
    }

    for (int i = 0; i < n; ++i) {
        TObject* obj = arr.At(i);

        if (!obj) {
            out.emplace_back(default_value);
            continue;
        }

        char* base = reinterpret_cast<char*>(obj);
        char* addr = base + offset;

        out.emplace_back(static_cast<T>(*reinterpret_cast<T*>(addr)));
    }

    return out;
}

} // namespace DelphesRDFHelper


int Delphes_N(const TClonesArray& arr) {
    return DelphesRDFHelper::N(arr);
}

RVec<int> Delphes_Get_int(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<int>(arr, class_name, member, -999);
}

RVec<unsigned int> Delphes_Get_uint(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<unsigned int>(arr, class_name, member, 0u);
}

RVec<long long> Delphes_Get_long64(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<long long>(arr, class_name, member, -999);
}

RVec<unsigned long long> Delphes_Get_ulong64(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<unsigned long long>(arr, class_name, member, 0ull);
}

RVec<float> Delphes_Get_float(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<float>(arr, class_name, member, -999.f);
}

RVec<double> Delphes_Get_double(
    const TClonesArray& arr,
    const std::string& class_name,
    const std::string& member
) {
    return DelphesRDFHelper::GetByOffset<double>(arr, class_name, member, -999.);
}
"""
)

    _DECLARED_DELPHES_RDF_HELPERS = True


def _sanitize_name(name):
    return (
        str(name)
        .replace(".", "_")
        .replace("[", "_")
        .replace("]", "_")
        .replace(" ", "_")
        .replace("-", "_")
    )


def _is_timber_analyzer(obj):

    return (
        hasattr(obj, "DataFrame")
        and hasattr(obj, "ActiveNode")
        and hasattr(obj, "Define")
        and hasattr(obj, "GetColumnNames")
        and hasattr(obj, "_eventsChain")
    )


def _get_existing_columns(analyzer_or_df):
    cols = analyzer_or_df.GetColumnNames()
    return set(str(c) for c in cols)


def _discover_delphes_collections_from_tree(tree):
    """
    Discover top-level Delphes TClonesArray branches.

    Returns:
        list[str]
    """
    collections = []

    branches = tree.GetListOfBranches()
    if not branches:
        return collections

    for br in branches:
        name = br.GetName()

        # Avoid split subbranches and ROOT size/helper branches.
        if "." in name:
            continue
        if name.endswith("_size"):
            continue

        class_name = br.GetClassName()

        # Delphes object collections are normally TClonesArray branches.
        if class_name == "TClonesArray":
            collections.append(name)

    return collections


def _discover_delphes_collections_from_chain(chain):
    tree = chain.GetTree()
    if tree:
        return _discover_delphes_collections_from_tree(tree)

    # If the chain has not loaded a tree yet, force-load the first entry.
    if chain.GetEntries() > 0:
        chain.LoadTree(0)
        tree = chain.GetTree()
        if tree:
            return _discover_delphes_collections_from_tree(tree)

    return []


def _discover_delphes_collections_from_file(input_files, tree_name="Delphes"):
    if isinstance(input_files, str):
        input_files = [input_files]

    f = ROOT.TFile.Open(input_files[0])
    if not f or f.IsZombie():
        raise RuntimeError(f"Could not open input file: {input_files[0]}")

    tree = f.Get(tree_name)
    if not tree:
        f.Close()
        raise RuntimeError(f"Could not find tree '{tree_name}' in {input_files[0]}")

    collections = _discover_delphes_collections_from_tree(tree)
    f.Close()

    return collections

def _get_clones_class_name(branch):
    """
    For a Delphes TClonesArray branch, return the contained class name,
    e.g. Jet, Electron, Muon, HepMCEvent, MissingET.

    Avoids reading an event and avoids obj.IsA(), which can segfault for
    fragile/missing dictionaries.
    """
    if not branch:
        return None

    # TBranchElement for TClonesArray usually has GetClonesName()
    if hasattr(branch, "GetClonesName"):
        try:
            cname = branch.GetClonesName()
            if cname:
                return str(cname)
        except Exception:
            pass

    # Sometimes the title contains something useful. This is fallback only.
    try:
        title = str(branch.GetTitle())
        # Often title can be like "Jet" or "TClonesArray"
        if title and title != "TClonesArray":
            return title
    except Exception:
        pass

    return None


def _discover_collections_and_classes_from_tree(tree):
    """
    Discover Delphes top-level TClonesArray branches and their contained class.

    Returns:
        dict:
            {
                "Jet": "Jet",
                "Electron": "Electron",
                "Event": "HepMCEvent",
                ...
            }
    """
    out = {}

    branches = tree.GetListOfBranches()
    if not branches:
        return out

    for br in branches:
        bname = str(br.GetName())

        if "." in bname:
            continue
        if bname.endswith("_size"):
            continue

        if str(br.GetClassName()) != "TClonesArray":
            continue

        cname = _get_clones_class_name(br)

        if cname:
            out[bname] = cname
        else:
            out[bname] = None

    return out


def _discover_collections_and_classes_from_chain(chain):
    tree = chain.GetTree()

    if not tree and chain.GetEntries() > 0:
        chain.LoadTree(0)
        tree = chain.GetTree()

    if not tree:
        return {}

    return _discover_collections_and_classes_from_tree(tree)


def _discover_members_from_class_name(class_name):
    """
    Discover scalar numeric members from a class name without creating or
    touching an actual object.

    Returns:
        list of tuples:
            (member_name, cpp_type, logical_type, rvec_value_type, getter)
    """
    if not class_name:
        return []

    cls = ROOT.TClass.GetClass(class_name) # type: ignore

    if not cls:
        print(f"[enable_delphes] WARNING: no TClass for {class_name}; skipping")
        return []

    return _discover_members_from_class(cls)



def _discover_members_from_class(cls):
    """
    Discover scalar numeric data members from a ROOT TClass.

    Returns:
        list of tuples:
            (member_name, cpp_type, logical_type, rvec_value_type, getter_function)
    """
    members = []

    if not cls:
        return members

    data_members = cls.GetListOfDataMembers()
    if not data_members:
        return members

    for dm in data_members:
        name = dm.GetName()
        cpp_type = dm.GetTypeName()

        # Skip arrays.
        if dm.GetArrayDim() > 0:
            continue
        if "[" in name or "]" in name:
            continue

        # Skip pointers and object-like things.
        if "*" in cpp_type:
            continue

        if cpp_type not in TYPE_MAP:
            continue

        logical_type, rvec_value_type, getter = TYPE_MAP[cpp_type]
        members.append((name, cpp_type, logical_type, rvec_value_type, getter))

    return members


def _build_chain_from_files(input_files, tree_name):
    if isinstance(input_files, str):
        input_files = [input_files]

    chain = ROOT.TChain(tree_name)
    for fname in input_files:
        chain.Add(fname)

    return chain


def _define_column_timber(analyzer, name, expr, nodetype="DelphesRVec"):
    """
    Add a column through TIMBER analyzer.Define so node tracking and
    CollectionOrganizer stay consistent.
    """
    analyzer.Define(name, expr, nodetype=nodetype)
    return analyzer


def _define_column_rdf(df, name, expr):
    """
    Plain RDataFrame path. RDataFrame.Define returns a new dataframe.
    """
    return df.Define(name, expr)


def enable_delphes(
    analyzer_or_df,
    input_files=None,
    tree_name=None,
    define_multiplicities=True,
    skip_existing=True,
    nodetype="DelphesRVec",
):
    """
    Lazily expose Delphes TClonesArray branches as NanoAOD-like RVec columns.

    Works with either:
      1. TIMBER analyzer
      2. plain ROOT.RDataFrame

    For TIMBER:
        a = analyzer("file.root", eventsTreeName="Delphes")
        a = enable_delphes_rvecs(a)
        a.SubCollection("GoodJet", "Jet", "Jet_PT > 30")
        a.Snapshot("all", "flat.root", "Events")

    For plain RDF:
        df = ROOT.RDataFrame("Delphes", "file.root")
        df = enable_delphes_rvecs(
            df,
            input_files="file.root",
            tree_name="Delphes",
        )

    Parameters
    ----------
    analyzer_or_df:
        TIMBER analyzer or ROOT.RDataFrame.

    input_files:
        Required for plain RDataFrame unless `chain` discovery is otherwise added.
        Optional for TIMBER, because the helper can use analyzer._eventsChain.

    tree_name:
        Input tree name. For TIMBER this defaults to analyzer._eventsTreeName.
        For Delphes this is usually "Delphes".

    Returns
    -------
    TIMBER analyzer if TIMBER input was given, otherwise a new ROOT.RDataFrame.
    """

    _declare_rdf_helpers()

    is_timber = _is_timber_analyzer(analyzer_or_df)

    if is_timber:
        analyzer = analyzer_or_df
        df = analyzer.DataFrame
        chain = analyzer._eventsChain

        if tree_name is None:
            tree_name = getattr(analyzer, "_eventsTreeName", "Delphes")

    else:
        analyzer = None
        df = analyzer_or_df

        if tree_name is None:
            tree_name = "Delphes"

        if input_files is None:
            raise ValueError(
                "For plain ROOT.RDataFrame usage, please pass input_files=... "
                "so Delphes collections/members can be discovered."
            )

        chain = _build_chain_from_files(input_files, tree_name)

    existing_columns = _get_existing_columns(analyzer if is_timber else df)

    # Discover collection -> contained-class-name from branch metadata.
    if is_timber:
        collection_classes = _discover_collections_and_classes_from_chain(chain)
    else:
        tmp_chain = _build_chain_from_files(input_files, tree_name)
        collection_classes = _discover_collections_and_classes_from_chain(tmp_chain)

    collections = list(collection_classes.keys())

    all_member_info = {}

    for coll in collections:
        class_name = collection_classes.get(coll, None)

        discovered = _discover_members_from_class_name(class_name)

        all_member_info[coll] = discovered


        if not discovered:
            print("  no scalar numeric members discovered")
        for member_name, cpp_type, logical_type, rvec_value_type, getter in discovered:
            print(f"  {member_name:<24} {cpp_type:<12} -> RVec<{rvec_value_type}>")

    # Define nCollection and Collection_member columns.
    if is_timber:
        out = analyzer
    else:
        out = df

    for coll in collections:
        safe_coll = _sanitize_name(coll)

        if define_multiplicities:
            n_name = f"n{safe_coll}"
            n_expr = f"Delphes_N({coll})"

            if skip_existing and n_name in existing_columns:
                print(f"Skipping existing column {n_name}")
            else:
 
                print(f"Defining {n_name}: {n_expr}")

                if is_timber:
                    out = _define_column_timber(out, n_name, n_expr, nodetype=nodetype)
                else:
                    out = _define_column_rdf(out, n_name, n_expr)

                existing_columns.add(n_name)

        for member_name, cpp_type, logical_type, rvec_value_type, getter in all_member_info[coll]:
            safe_member = _sanitize_name(member_name)
            out_name = f"{safe_coll}_{safe_member}"

            class_name = collection_classes.get(coll, None)

            if not class_name:
                print(f"Skipping {out_name}: unknown contained class")
                continue
            expr = f'{getter}({coll}, "{class_name}", "{member_name}")'
            
            if skip_existing and out_name in existing_columns:
                print(f"Skipping existing column {out_name}")
                continue


            print(f"Defining {out_name}: {expr}")

            if is_timber:
                out = _define_column_timber(out, out_name, expr, nodetype=nodetype)
            else:
                out = _define_column_rdf(out, out_name, expr)

            existing_columns.add(out_name)

    return out


# call this upon the module's import
_load_delphes(load_lib=True, declare_delphes_classes=True, declare_exroot=True)