#include "doctest/doctest.h"
#include "ApprovalTests/core/ApprovalNamer.h"
#include "ApprovalTests/namers/SeparateApprovedAndReceivedDirectoriesNamer.h"
#include "ApprovalTests/namers/NamerFactory.h"
#include "ApprovalTests/utilities/Path.h"
#include "ApprovalTests/utilities/SystemUtils.h"

//#include <filesystem>
#include <iostream>
#include <memory>

using namespace ApprovalTests;

class CustomNamer;

class PathBasedOption
{
public:
    using PathFunction = std::function<Path(CustomNamer)>;
};

class CustomNamer : public ApprovalNamer
{
public:
    using PathFunction = PathBasedOption::PathFunction;

private:
    ApprovalTestNamer namer_;

    PathBasedOption testFolderOption_;
    PathFunction testFolder_ = [](CustomNamer namer) {
        return Path(namer.namer_.getTestSourceDirectory());
    };

    PathFunction testFolderForApproved_ = [](CustomNamer namer) {
        return namer.getTestFolder();
    };

public:
    Path getTestFolder() const
    {
        return testFolder_(*this);
    }

    CustomNamer withTestFolder(std::string value)
    {
        testFolder_ = [value](CustomNamer /*namer*/) { return Path(value); };
        return *this;
    }

    CustomNamer withTestFolder(PathFunction newMethod)
    {
        testFolder_ = newMethod;
        return *this;
    }

    Path getTestFolderForApproved() const
    {
        return testFolderForApproved_(*this);
    }

    CustomNamer withTestFolderForApproved(std::string value)
    {
        testFolderForApproved_ = [value](CustomNamer /*namer*/) { return Path(value); };
        return *this;
    }

    CustomNamer withTestFolderForApproved(PathFunction newMethod)
    {
        testFolderForApproved_ = newMethod;
        return *this;
    }

    Path getRelativePathOfSourceDirectoryFromSourceRootForApproved() const
    {
        return Path(namer_.getRelativePathOfSourceDirectoryFromSourceRootForApproved());
    }

    std::string getFileNameAndTestName() const
    {
        return namer_.getFileName() + "." + namer_.getTestName();
    }

    virtual std::string getApprovedFile(std::string extensionWithDot) const override
    {
        return (getTestFolderForApproved() /
                    getRelativePathOfSourceDirectoryFromSourceRootForApproved() /
                    getFileNameAndTestName() +
                ".approved" + extensionWithDot)
            .toString();
    }

    virtual std::string getReceivedFile(std::string /*extensionWithDot*/) const override
    {
        return "my.received";
    }
};

namespace
{
    static std::string normalize(const std::string& custom)
    {
        return StringUtils::replaceAll(custom, "\\", "/");
    }
}

TEST_CASE("Default Behaviour")
{
    auto result = Approvals::getDefaultNamer()->getApprovedFile(".txt");
    auto custom = CustomNamer().getApprovedFile(".txt");
    REQUIRE(result == custom);
}

TEST_CASE("Behaviour with custom directory")
{
    auto custom =
        CustomNamer()
            .withTestFolder([](auto /*namer*/) { return Path("custom/location"); })
            .getApprovedFile(".txt");
    REQUIRE("custom/location/approval_tests/"
            "CustomNamerTests.Behaviour_with_custom_directory.approved.txt" ==
            normalize(custom));

    auto custom2 =
        CustomNamer().withTestFolder("custom/location").getApprovedFile(".txt");
    REQUIRE(custom == custom2);
}

TEST_CASE("Test Every Customization")
{
    auto custom = CustomNamer()
                      .withTestFolder("custom/location")
                      .withTestFolderForApproved([](auto that) {
                          return that.getTestFolder() / "approved_files";
                      })
                      .getApprovedFile(".txt");
    REQUIRE("custom/location/approved_files/approval_tests/"
            "CustomNamerTests.Test_Every_Customization.approved.txt" ==
            normalize(custom));
}

// TODO Better names for methods
// TODO Finish up all the rest of the seams
// TODO Get this to work with received file
// TODO Make CustomNamer with... non-mutating
// TODO Use Doxygen grouping to group related methods together for docs
// TODO Move CustomNamer to .h and .cpp
// TODO Move Path impl to .cpp
// TODO Review usability
// TODO Consider user scenarios
// TODO Try it on an embedded device - or ask someone else to!
// TODO Revisit our documentation
