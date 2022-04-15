## heart-engine-playground | [![CI](https://github.com/growlitheharpo/heart-engine-playground/workflows/CI/badge.svg)](https://github.com/growlitheharpo/heart-engine-playground/actions?query=workflow%3ACI)

This is a tech playground for a "game engine" called Heart.

Notable components include **heart-codegen** and the deserialization component of heart-core; these two items play off each other (as well as entt and rapidjson) to create powerful loading capabilities with minimal markup. Check out [the unit test for codegen](heart/heart-test/src/codegen.cpp) for an example. The output of this process can be found in the [artifacts for the CI](https://github.com/growlitheharpo/heart-engine-playground/actions?query=workflow%3ACI).

Graphics are driven by SFML. Premake is used for project generation.

You may use, distribute, and modify this code under the terms of its **modified** BSD-3-Clause [license](license.md). Use for any commercial purposes is prohibited.
