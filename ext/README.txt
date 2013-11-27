This sparks/ext directory contains externally developed dependencies.

Note that cpplog and FreeTypeGL have been modified from their original distributions, so caution is needed on upgrading to new versions.

Some of these libraries could all be removed and replaced with ExternalProject_Add directives for cmake, but others, e.g. AscensionTech, are not standard repositories, while others require modifications or particular revisions.

