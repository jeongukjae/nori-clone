# Compare with original nori

To compare outputs with original nori, you can run as follows.

```sh
bazel run //tools/comparison:compare_outputs
```

In most cases, the outputs are the same, but sometimes you can see that they have different values, such as a dot and a comma.

If you want to check the specific cases, update [`data.txt`](./data.txt) and run `compare_outputs` command again.
