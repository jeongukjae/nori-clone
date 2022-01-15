load("@bazel_skylib//lib:shell.bzl", "shell")

def _google_java_format_impl_factory(ctx, test_rule = False):
    args = ["--aosp"]
    if test_rule:
        args.append("--set-exit-if-changed")
    else:
        args.append("--replace")

    runner_files = depset(ctx.files._runner).to_list()
    if len(runner_files) != 1:
        fail("length _runner's file should be 1")

    jar_file = runner_files[0]
    executable = ctx.actions.declare_file(ctx.attr.name + ".bash")

    exclude_patterns_str = " ".join(["\\! -path %s" % shell.quote(pattern) for pattern in ctx.attr.exclude_patterns])
    substitutions = {
        "@@ARGS@@": shell.array_literal(args),
        "@@JAR@@": shell.quote(jar_file.short_path),
        "@@EXCLUDE_PATTERNS@@": exclude_patterns_str,
    }
    ctx.actions.expand_template(
        template = ctx.file._template,
        output = executable,
        substitutions = substitutions,
        is_executable = True,
    )
    runfiles = [jar_file]
    if test_rule:
        runfiles.extend(ctx.files.srcs)

    return DefaultInfo(
        files = depset([executable]),
        executable = executable,
        runfiles = ctx.runfiles(files = runfiles),
    )

def _get_attrs_for_google_java_format(test_rule = False):
    attrs = {
        "exclude_patterns": attr.string_list(allow_empty = True),
        "_runner": attr.label(
            default = "@com_github_google_google_java_format//jar",
            cfg = "host",
        ),
        "_template": attr.label(
            default = "//tools/lint/google_java_format:google_java_format.template.bash",
            allow_single_file = True,
        ),
    }

    if test_rule:
        attrs.update({
            "srcs": attr.label_list(
                allow_empty = False,
                allow_files = [".java"],
            ),
        })

    return attrs

def _google_java_format_impl(ctx):
    return [_google_java_format_impl_factory(ctx, False)]

google_java_format = rule(
    attrs = _get_attrs_for_google_java_format(False),
    implementation = _google_java_format_impl,
    executable = True,
)

def _google_java_format_test_impl(ctx):
    return [_google_java_format_impl_factory(ctx, True)]

google_java_format_test = rule(
    attrs = _get_attrs_for_google_java_format(True),
    implementation = _google_java_format_test_impl,
    test = True,
)
