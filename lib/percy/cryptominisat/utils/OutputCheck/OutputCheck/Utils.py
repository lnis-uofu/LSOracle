def isA(instance, typeList):
    """
        Return true if ``instance`` is an instance of any the Directive
        types in ``typeList``
    """
    return any(map(lambda iType: isinstance(instance,iType), typeList))
