/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"

#import "DragAndDropSimulator.h"
#import "PlatformUtilities.h"

#if WK_API_ENABLED && ENABLE(DRAG_SUPPORT)

TEST(DragAndDropTests, DragImageLocationForLinkInSubframe)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 400, 400)]);
    [[simulator webView] synchronouslyLoadTestPageNamed:@"link-in-iframe-and-input"];
    [simulator runFrom:CGPointMake(200, 375) to:CGPointMake(200, 125)];

    EXPECT_WK_STREQ("https://www.apple.com/", [[simulator webView] stringByEvaluatingJavaScript:@"document.querySelector('input').value"]);

#if PLATFORM(MAC)
    EXPECT_TRUE(NSPointInRect([simulator initialDragImageLocationInView], NSMakeRect(0, 250, 400, 250)));
#endif
}

#if ENABLE(INPUT_TYPE_COLOR)

TEST(DragAndDropTests, ColorInputToColorInput)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 320, 500)]);
    auto webView = [simulator webView];

    [webView synchronouslyLoadTestPageNamed:@"color-drop"];
    [simulator runFrom:CGPointMake(50, 50) to:CGPointMake(150, 50)];
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").value"]);
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").value"]);
}

TEST(DragAndDropTests, ColorInputToDisabledColorInput)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 320, 500)]);
    auto webView = [simulator webView];

    [webView synchronouslyLoadTestPageNamed:@"color-drop"];
    [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").disabled = true"];
    [simulator runFrom:CGPointMake(50, 50) to:CGPointMake(150, 50)];
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").value"]);
    EXPECT_WK_STREQ(@"#ff0000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").value"]);
}

TEST(DragAndDropTests, DisabledColorInputToColorInput)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 320, 500)]);
    auto webView = [simulator webView];

    [webView synchronouslyLoadTestPageNamed:@"color-drop"];
    [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").disabled = true"];
    [simulator runFrom:CGPointMake(50, 50) to:CGPointMake(150, 50)];
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").value"]);
    EXPECT_WK_STREQ(@"#ff0000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").value"]);
}

TEST(DragAndDropTests, ReadOnlyColorInputToReadOnlyColorInput)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 320, 500)]);
    auto webView = [simulator webView];

    [webView synchronouslyLoadTestPageNamed:@"color-drop"];
    [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").readOnly = true"];
    [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").readOnly = true"];
    [simulator runFrom:CGPointMake(50, 50) to:CGPointMake(150, 50)];
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drag-target\").value"]);
    EXPECT_WK_STREQ(@"#000000", [webView stringByEvaluatingJavaScript:@"document.getElementById(\"drop-target\").value"]);
}

TEST(DragAndDropTests, ColorInputEvents)
{
    auto simulator = adoptNS([[DragAndDropSimulator alloc] initWithWebViewFrame:CGRectMake(0, 0, 320, 500)]);
    auto webView = [simulator webView];

    [webView synchronouslyLoadTestPageNamed:@"color-drop"];

    __block bool changeEventFired = false;
    [webView performAfterReceivingMessage:@"change" action:^() {
        changeEventFired = true;
    }];

    __block bool inputEventFired = false;
    [webView performAfterReceivingMessage:@"input" action:^() {
        inputEventFired = true;
    }];

    [simulator runFrom:CGPointMake(50, 50) to:CGPointMake(150, 50)];
    TestWebKitAPI::Util::run(&inputEventFired);
    TestWebKitAPI::Util::run(&changeEventFired);
}

#endif

#endif // WK_API_ENABLED && ENABLE(DRAG_SUPPORT)
