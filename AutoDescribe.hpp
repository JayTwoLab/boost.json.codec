#pragma once

#ifndef BOOST_AUTO_DESCRIBE_HPP
#define BOOST_AUTO_DESCRIBE_HPP

// AutoDescribe.hpp
// A utility that generates both the struct definition and BOOST_DESCRIBE_STRUCT automatically
// by defining the "field sequence" only once.

#include <boost/preprocessor.hpp>
#include <boost/describe.hpp>

// (name, type, init) -> "type name{init};"
#define ADH_DECLARE_MEMBER(r, data, elem)                         \
BOOST_PP_TUPLE_ELEM(3, 1, elem) BOOST_PP_TUPLE_ELEM(3, 0, elem){  \
    BOOST_PP_TUPLE_ELEM(3, 2, elem)                               \
};

// Extract the list of member names for use in describe
#define ADH_EXTRACT_NAME(r, data, i, elem)                       \
BOOST_PP_COMMA_IF(i) BOOST_PP_TUPLE_ELEM(3, 0, elem)

// Generate both the struct and its describe at once
#define ADH_MAKE_STRUCT_AND_DESCRIBE(name, fields_seq)            \
    struct name {                                                 \
        BOOST_PP_SEQ_FOR_EACH(ADH_DECLARE_MEMBER, _, fields_seq)  \
};                                                                \
    BOOST_DESCRIBE_STRUCT(                                        \
                                                                  name,                                                   \
                                                                  (),                                                     \
                                                                  (BOOST_PP_SEQ_FOR_EACH_I(ADH_EXTRACT_NAME, _, fields_seq))       \
        )

#endif // AUTODESCRIBE_HPP
