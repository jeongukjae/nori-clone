load("@bazel_skylib//lib:shell.bzl", "shell")

def _clang_format_impl_factory(ctx, test_rule = False):
    args = ["-i"]
    if test_rule:
        args.extend(["--dry-run", "--Werror"])

    executable = ctx.actions.declare_file(ctx.attr.name + ".bash")

    exclude_patterns_str = " ".join(["\\! -path %s" % shell.quote(pattern) for pattern in ctx.attr.exclude_patterns])
    substitutions = {
        "@@ARGS@@": shell.array_literal(args),
        "@@CMD@@": ctx.attr._cmd,
        "@@EXCLUDE_PATTERNS@@": exclude_patterns_str,
    }
    ctx.actions.expand_template(
        template = ctx.file._template,
        output = executable,
        substitutions = substitutions,
        is_executable = True,
    )

    return DefaultInfo(
        files = depset([executable]),
        executable = executable,
    )

def _clang_format_impl(ctx):
    return [_clang_format_impl_factory(ctx, False)]

clang_format = rule(
    attrs = {
        "exclude_patterns": attr.string_list(allow_empty = True),
        "_cmd": attr.string(default = "clang-format"),
        "_template": attr.label(
            default = "//tools/lint:clang_format.template.bash",
            allow_single_file = True,
        ),
    },
    implementation = _clang_format_impl,
    executable = True,
)

def _clang_format_test_impl(ctx):
    return [_clang_format_impl_factory(ctx, True)]

clang_format_test = rule(
    attrs = {
        "exclude_patterns": attr.string_list(allow_empty = True),
        "_cmd": attr.string(default = "clang-format"),
        "_template": attr.label(
            default = "//tools/lint:clang_format.template.bash",
            allow_single_file = True,
        ),
    },
    implementation = _clang_format_test_impl,
    test = True,
)
