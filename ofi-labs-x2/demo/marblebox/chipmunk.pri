CONFIG(release): QMAKE_CFLAGS += -DNDEBUG

INCLUDEPATH += $$PWD/chipmunk/include/chipmunk

QMAKE_CFLAGS += -std=c99

HEADERS += chipmunk/include/chipmunk/constraints/util.h \
    chipmunk/include/chipmunk/constraints/cpSlideJoint.h \
    chipmunk/include/chipmunk/constraints/cpSimpleMotor.h \
    chipmunk/include/chipmunk/constraints/cpRotaryLimitJoint.h \
    chipmunk/include/chipmunk/constraints/cpRatchetJoint.h \
    chipmunk/include/chipmunk/constraints/cpPivotJoint.h \
    chipmunk/include/chipmunk/constraints/cpPinJoint.h \
    chipmunk/include/chipmunk/constraints/cpGrooveJoint.h \
    chipmunk/include/chipmunk/constraints/cpGearJoint.h \
    chipmunk/include/chipmunk/constraints/cpDampedSpring.h \
    chipmunk/include/chipmunk/constraints/cpDampedRotarySpring.h \
    chipmunk/include/chipmunk/constraints/cpConstraint.h \
    chipmunk/include/chipmunk/cpVect.h \
    chipmunk/include/chipmunk/cpSpaceHash.h \
    chipmunk/include/chipmunk/cpSpace.h \
    chipmunk/include/chipmunk/cpShape.h \
    chipmunk/include/chipmunk/cpPolyShape.h \
    chipmunk/include/chipmunk/cpHashSet.h \
    chipmunk/include/chipmunk/cpCollision.h \
    chipmunk/include/chipmunk/cpBody.h \
    chipmunk/include/chipmunk/cpBB.h \
    chipmunk/include/chipmunk/cpArray.h \
    chipmunk/include/chipmunk/cpArbiter.h \
    chipmunk/include/chipmunk/chipmunk.h \
    chipmunk/include/chipmunk/chipmunk_unsafe.h \
    chipmunk/include/chipmunk/chipmunk_types.h \
    chipmunk/include/chipmunk/chipmunk_ffi.h \
    chipmunk/src/prime.h

SOURCES += chipmunk/src/constraints/cpSlideJoint.c \
    chipmunk/src/constraints/cpSimpleMotor.c \
    chipmunk/src/constraints/cpRotaryLimitJoint.c \
    chipmunk/src/constraints/cpRatchetJoint.c \
    chipmunk/src/constraints/cpPivotJoint.c \
    chipmunk/src/constraints/cpPinJoint.c \
    chipmunk/src/constraints/cpGrooveJoint.c \
    chipmunk/src/constraints/cpGearJoint.c \
    chipmunk/src/constraints/cpDampedSpring.c \
    chipmunk/src/constraints/cpDampedRotarySpring.c \
    chipmunk/src/constraints/cpConstraint.c \
    chipmunk/src/cpVect.c \
    chipmunk/src/cpSpaceHash.c \
    chipmunk/src/cpSpace.c \
    chipmunk/src/cpShape.c \
    chipmunk/src/cpPolyShape.c \
    chipmunk/src/cpHashSet.c \
    chipmunk/src/cpCollision.c \
    chipmunk/src/cpBody.c \
    chipmunk/src/cpBB.c \
    chipmunk/src/cpArray.c \
    chipmunk/src/cpArbiter.c \
    chipmunk/src/chipmunk.c

