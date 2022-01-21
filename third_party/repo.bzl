def clean_dep(label):
    return str(Label(label))

def clean_deps(labels):
    return [str(Label(label)) for label in labels]
