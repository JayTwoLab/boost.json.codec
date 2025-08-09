#pragma once

#ifndef AUTODESCRIBE_HPP
#define AUTODESCRIBE_HPP

// AutoDescribe.hpp
// "필드 시퀀스"를 한 번만 정의하면 구조체와 BOOST_DESCRIBE_STRUCT를 자동 생성하는 유틸

#include <boost/preprocessor.hpp>
#include <boost/describe.hpp>

// (name, type, init) -> "type name{init};"
#define ADH_DECLARE_MEMBER(r, data, elem)                                           \
BOOST_PP_TUPLE_ELEM(3, 1, elem) BOOST_PP_TUPLE_ELEM(3, 0, elem){                \
    BOOST_PP_TUPLE_ELEM(3, 2, elem)                                             \
};

// describe에 넣을 멤버 이름 목록 추출
#define ADH_EXTRACT_NAME(r, data, i, elem)                                          \
BOOST_PP_COMMA_IF(i) BOOST_PP_TUPLE_ELEM(3, 0, elem)

// 구조체와 describe를 한 번에 생성
#define ADH_MAKE_STRUCT_AND_DESCRIBE(name, fields_seq)                              \
    struct name {                                                                   \
        BOOST_PP_SEQ_FOR_EACH(ADH_DECLARE_MEMBER, _, fields_seq)                    \
};                                                                              \
    BOOST_DESCRIBE_STRUCT(                                                          \
                                                                                    name,                                                                       \
                                                                                    (),                                                                         \
                                                                                    (BOOST_PP_SEQ_FOR_EACH_I(ADH_EXTRACT_NAME, _, fields_seq))                  \
        )


#endif // AUTODESCRIBE_HPP
