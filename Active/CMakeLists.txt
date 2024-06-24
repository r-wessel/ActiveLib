cmake_minimum_required(VERSION 3.20)

IF (WIN32)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
ENDIF()

add_library(
    ActiveLib

    # Attribute
    Attribute/Colour.cpp

    # Environment
    Environment/App.cpp

    # Event
    Event/Publisher.cpp

    # File
    File/Directory.cpp
    File/File.cpp
    File/Path.cpp
    File/Interface/FSNode.cpp

    # Geometry
    Geometry/Anchor2D.cpp
    Geometry/Arc.cpp
    Geometry/Box.cpp
    Geometry/Faceter.cpp
    Geometry/Leveller.cpp
    Geometry/Line.cpp
    Geometry/LinEquation.cpp
    Geometry/Matrix3x3.cpp
    Geometry/Matrix4x4.cpp
    Geometry/Plane.cpp
    Geometry/Point.cpp
    Geometry/PolyEdge.cpp
    Geometry/Polygon.cpp
    Geometry/PolyPoint.cpp
    Geometry/Rotater.cpp
    Geometry/Vector3.cpp
    Geometry/Vector4.cpp
    Geometry/XList.cpp
    Geometry/XPoint.cpp

    # Serialize
    Serialise/Document/Object.cpp
    Serialise/Generic/Base64Transport.cpp
    Serialise/Generic/HexTransport.cpp
    Serialise/Inventory/Inventory.cpp
    Serialise/Item/Item.cpp
    Serialise/Item/Wrapper/AnyValueWrap.cpp
    Serialise/JSON/JSONTransport.cpp
    Serialise/Package/Pair.cpp
    Serialise/XML/Item/XMLDateTime.cpp
    Serialise/XML/Package/Wrapper/Geometry/XMLPoint.cpp
    Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.cpp
    Serialise/XML/Package/Wrapper/XMLValueSetting.cpp
    Serialise/XML/Package/XMLProlog.cpp
    Serialise/XML/XMLTransport.cpp

    # Setting
    Setting/SettingList.cpp
	Setting/Values/Measurement/AngleValue.cpp
    Setting/Values/Measurement/Units/AngleUnit.cpp
    Setting/Values/Measurement/Units/AreaUnit.cpp
    Setting/Values/Measurement/Units/LengthUnit.cpp
    Setting/Values/Measurement/Units/MassUnit.cpp
    Setting/Values/Value.cpp
    Setting/ValueSetting.cpp

    # Utility
    Utility/BufferIn.cpp
    Utility/BufferOut.cpp
    Utility/DataFormat.cpp
    Utility/Guid.cpp
    Utility/Memory.cpp
    Utility/String.cpp
    Utility/TextEncoding.cpp
    Utility/Time.cpp
    Utility/UniqueString.cpp
)

add_library(${namespace}ActiveLib ALIAS ActiveLib)

target_sources(
    ActiveLib PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}
    FILES

    # Attribute
	${CMAKE_CURRENT_SOURCE_DIR}/Attribute/Colour.h
	${CMAKE_CURRENT_SOURCE_DIR}/Attribute/Finish.h
	${CMAKE_CURRENT_SOURCE_DIR}/Attribute/Pen.h
	${CMAKE_CURRENT_SOURCE_DIR}/Attribute/TextStyle.h

    # Container
	${CMAKE_CURRENT_SOURCE_DIR}/Container/HashMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Container/List.h
	${CMAKE_CURRENT_SOURCE_DIR}/Container/Map.h
	${CMAKE_CURRENT_SOURCE_DIR}/Container/MultiMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Container/Vector.h

    # Environment
	${CMAKE_CURRENT_SOURCE_DIR}/Environment/App.h

    # Event
	${CMAKE_CURRENT_SOURCE_DIR}/Event/Event.h
	${CMAKE_CURRENT_SOURCE_DIR}/Event/Message.h
	${CMAKE_CURRENT_SOURCE_DIR}/Event/Participant.h
	${CMAKE_CURRENT_SOURCE_DIR}/Event/PostBox.h
	${CMAKE_CURRENT_SOURCE_DIR}/Event/Publisher.h
	${CMAKE_CURRENT_SOURCE_DIR}/Event/Subscriber.h

    # File
	${CMAKE_CURRENT_SOURCE_DIR}/File/Directory.h
	${CMAKE_CURRENT_SOURCE_DIR}/File/File.h
	${CMAKE_CURRENT_SOURCE_DIR}/File/Interface/FSNode.h
	${CMAKE_CURRENT_SOURCE_DIR}/File/Interface/IOBase.h
	${CMAKE_CURRENT_SOURCE_DIR}/File/Path.h

    # Geometry
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Anchor2D.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Arc.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Box.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Faceter.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Leveller.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Line.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/LinEquation.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Matrix3x3.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Matrix4x4.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Plane.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Point.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/PolyEdge.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Polygon.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/PolyPoint.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Position.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Rotater.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Rotation.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Vector3.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/Vector4.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/XInfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/XList.h
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/XPoint.h

    # Serialise
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Cargo.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Document/Handler.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Document/Object.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Document/Transportable.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Generic/Base64Transport.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Generic/HexTransport.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Inventory/Entry.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Inventory/Identity.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Inventory/Inventory.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/Encoding.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/Item.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/UnknownItem.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/Wrapper/AnyValueWrap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/Wrapper/ItemWrap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Item/Wrapper/ValueWrap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/JSON/JSONTransport.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Package/Package.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Package/PackageWrap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Package/Pair.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Package/Unknown.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/Transport.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Item/XMLDateTime.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Item/XMLLength.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Item/XMLNumber.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Package/Wrapper/Geometry/XMLPoint.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Package/Wrapper/Geometry/XMLPolygon.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Package/Wrapper/XMLValueSetting.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/Package/XMLProlog.h
	${CMAKE_CURRENT_SOURCE_DIR}/Serialise/XML/XMLTransport.h

    # Setting
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Setting.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/SettingList.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/BoolValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/DoubleValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/GuidValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Int32Value.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Int64Value.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/AngleValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/AreaValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/LengthValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/MassValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/MeasuredValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/Units/AngleUnit.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/Units/AreaUnit.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/Units/LengthUnit.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/Units/MassUnit.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Measurement/Units/Unit.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/NullValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/StringValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/TimeValue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/UInt32Value.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/Value.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/Values/ValueBase.h
	${CMAKE_CURRENT_SOURCE_DIR}/Setting/ValueSetting.h

    # Utility
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/BufferIn.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/BufferOut.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/Cloner.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/DataFormat.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/Defer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/Guid.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/MathFunctions.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/Memory.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/NameID.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/StackBufferOut.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/String.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/TextEncoding.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/Time.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/UniqueString.h
)

IF (WIN32)
	target_compile_definitions(ActiveLib PRIVATE
        WINDOWS
        _LIB
    )
ENDIF()

set_target_properties(ActiveLib PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME "${ActiveLib_lib_name}"
)

target_include_directories(
    ActiveLib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# ---- library installation ----
install(
    TARGETS ActiveLib
    EXPORT "${targets_export_name}"
    FILE_SET HEADERS
    INCLUDES DESTINATION "${include_install_dir}"
)
