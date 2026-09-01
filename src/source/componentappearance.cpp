#include "componentappearance.h"

#include <QByteArray>

#include <cstring>


void ComponentAppearance::addLine(
    const Position& begin,
    const Position& end,
    const QColor& color,
    double width
)
{
    lines.emplace_back(begin, end, color, width);
}


void ComponentAppearance::addCurve(
    const Position& begin,
    const Position& control1,
    const Position& control2,
    const Position& end,
    const QColor& color,
    double width
)
{
    curves.emplace_back(
        begin,
        control1,
        control2,
        end,
        color,
        width
    );
}


void ComponentAppearance::addLabel(
    const QString& text,
    const Position& position,
    const QColor& color,
    int fontSize
)
{
    labels.emplace_back(
        text,
        position,
        color,
        fontSize
    );
}


void ComponentAppearance::setAnchor(const Position& position)
{
    anchor = position;
}


namespace
{
    uint32_t floatToUint32(double value)
    {
        float narrowed = static_cast<float>(value);
        uint32_t bits = 0;
        std::memcpy(&bits, &narrowed, sizeof(bits));
        return bits;
    }

    double uint32ToFloat(uint32_t bits)
    {
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));
        return static_cast<double>(value);
    }

    
    
    
    

    size_t packedStringUint32s(const QString& text)
    {
        const auto bytes = text.toUtf8();
        return 1 + (static_cast<size_t>(bytes.size()) + 3) / 4;
    }

    size_t writePackedString(uint32_t* address, const QString& text)
    {
        const auto bytes = text.toUtf8();
        uint32_t* start = address;

        *address++ = static_cast<uint32_t>(bytes.size());

        uint32_t word = 0;
        uint32_t shift = 0;

        for (unsigned char c : bytes)
        {
            word |= (static_cast<uint32_t>(c) << (shift * 8));

            if (shift == 3)
            {
                *address++ = word;
                word = 0;
                shift = 0;
            }
            else
            {
                ++shift;
            }
        }

        if (shift != 0)
            *address++ = word;

        return static_cast<size_t>(address - start);
    }

    QString readPackedString(
        const std::function<uint32_t(bool)>& readU32
    )
    {
        const uint32_t length = readU32(true);

        QByteArray bytes;
        bytes.reserve(static_cast<int>(length));

        for (uint32_t i = 0; i < length; ++i)
        {
            const unsigned char c =
                (readU32(false) >> ((i % 4) * 8)) & 0xff;

            bytes.push_back(static_cast<char>(c));

            if (i % 4 == 3)
                readU32(true);
        }

        if (length % 4 != 0)
            readU32(true);

        return QString::fromUtf8(bytes);
    }
}


size_t ComponentAppearance::getUint32sToSave() const
{
    
    size_t count = 2;

    
    count += 1 + lines.size() * 6;

    
    count += 1 + curves.size() * 10;

    
    count += 1;
    for (const auto& label : labels)
    {
        count += 2 + 1 + 1 + packedStringUint32s(label.text);
    }

    return count;
}


size_t ComponentAppearance::saveToAddress(uint32_t* address) const
{
    uint32_t* start = address;

    *address++ = static_cast<uint32_t>(anchor.x);
    *address++ = static_cast<uint32_t>(anchor.y);

    *address++ = static_cast<uint32_t>(lines.size());

    for (const auto& line : lines)
    {
        *address++ = static_cast<uint32_t>(line.begin.x);
        *address++ = static_cast<uint32_t>(line.begin.y);
        *address++ = static_cast<uint32_t>(line.end.x);
        *address++ = static_cast<uint32_t>(line.end.y);
        *address++ = line.color.rgba();
        *address++ = floatToUint32(line.width);
    }

    *address++ = static_cast<uint32_t>(curves.size());

    for (const auto& curve : curves)
    {
        *address++ = static_cast<uint32_t>(curve.begin.x);
        *address++ = static_cast<uint32_t>(curve.begin.y);
        *address++ = static_cast<uint32_t>(curve.control1.x);
        *address++ = static_cast<uint32_t>(curve.control1.y);
        *address++ = static_cast<uint32_t>(curve.control2.x);
        *address++ = static_cast<uint32_t>(curve.control2.y);
        *address++ = static_cast<uint32_t>(curve.end.x);
        *address++ = static_cast<uint32_t>(curve.end.y);
        *address++ = curve.color.rgba();
        *address++ = floatToUint32(curve.width);
    }

    *address++ = static_cast<uint32_t>(labels.size());

    for (const auto& label : labels)
    {
        *address++ = static_cast<uint32_t>(label.position.x);
        *address++ = static_cast<uint32_t>(label.position.y);
        *address++ = label.color.rgba();
        *address++ = static_cast<uint32_t>(label.fontSize);
        address += writePackedString(address, label.text);
    }

    return static_cast<size_t>(address - start);
}


void ComponentAppearance::loadFromReader(
    const std::function<uint32_t(bool)>& readU32
)
{
    anchor.x = static_cast<int>(readU32(true));
    anchor.y = static_cast<int>(readU32(true));

    const uint32_t lineCount = readU32(true);

    lines.clear();
    lines.reserve(lineCount);

    for (uint32_t i = 0; i < lineCount; ++i)
    {
        ComponentLine line;

        line.begin.x = static_cast<int>(readU32(true));
        line.begin.y = static_cast<int>(readU32(true));
        line.end.x = static_cast<int>(readU32(true));
        line.end.y = static_cast<int>(readU32(true));
        line.color = QColor::fromRgba(readU32(true));
        line.width = uint32ToFloat(readU32(true));

        lines.push_back(line);
    }

    const uint32_t curveCount = readU32(true);

    curves.clear();
    curves.reserve(curveCount);

    for (uint32_t i = 0; i < curveCount; ++i)
    {
        ComponentCurve curve;

        curve.begin.x = static_cast<int>(readU32(true));
        curve.begin.y = static_cast<int>(readU32(true));
        curve.control1.x = static_cast<int>(readU32(true));
        curve.control1.y = static_cast<int>(readU32(true));
        curve.control2.x = static_cast<int>(readU32(true));
        curve.control2.y = static_cast<int>(readU32(true));
        curve.end.x = static_cast<int>(readU32(true));
        curve.end.y = static_cast<int>(readU32(true));
        curve.color = QColor::fromRgba(readU32(true));
        curve.width = uint32ToFloat(readU32(true));

        curves.push_back(curve);
    }

    const uint32_t labelCount = readU32(true);

    labels.clear();
    labels.reserve(labelCount);

    for (uint32_t i = 0; i < labelCount; ++i)
    {
        ComponentLabel label;

        label.position.x = static_cast<int>(readU32(true));
        label.position.y = static_cast<int>(readU32(true));
        label.color = QColor::fromRgba(readU32(true));
        label.fontSize = static_cast<int>(readU32(true));
        label.text = readPackedString(readU32);

        labels.push_back(label);
    }
}