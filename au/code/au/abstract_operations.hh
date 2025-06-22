// Copyright 2025 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "au/magnitude.hh"

namespace au {
namespace detail {

//
// `OpInput<Op>` and `OpOutput<Op>` are the input and output types of an operation.
//
template <typename Op>
struct OpInputImpl;
template <typename Op>
using OpInput = typename OpInputImpl<Op>::type;

template <typename Op>
struct OpOutputImpl;
template <typename Op>
using OpOutput = typename OpOutputImpl<Op>::type;

//
// `StaticCast<T, U>` represents an operation that converts from `T` to `U` via `static_cast`.
//
template <typename T, typename U>
struct StaticCast {};

//
// `MultiplyTypeBy<T, M>` represents an operation that multiplies a value of type `T` by the
// magnitude `M`.
//
template <typename T, typename M>
struct MultiplyTypeBy {};

//
// `OpSequence<Ops...>` represents an ordered sequence of operations.
//
// We require that the output type of each operation is the same as the input type of the next one
// (see below for `OpInput` and `OpOutput`).
//
template <typename... Ops>
struct OpSequence {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`abstract_operations.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

template <typename T, typename U>
struct OpInputImpl<StaticCast<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<StaticCast<T, U>> : stdx::type_identity<U> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

template <typename Op, typename... Ops>
struct OpInputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpInput<Op>> {};

template <typename Op, typename... Ops>
struct OpOutputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpOutput<OpSequence<Ops...>>> {};
template <typename OnlyOp>
struct OpOutputImpl<OpSequence<OnlyOp>> : stdx::type_identity<OpOutput<OnlyOp>> {};

}  // namespace detail
}  // namespace au
