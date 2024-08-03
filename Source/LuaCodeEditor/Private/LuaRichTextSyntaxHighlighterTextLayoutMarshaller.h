#pragma once

#include "LuaCodeEditorStyle.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"

class FLuaRichTextSyntaxHighlighterTextLayoutMarshaller : public FSyntaxHighlighterTextLayoutMarshaller
{
public:

	struct FSyntaxTextStyle
	{
		FSyntaxTextStyle()
			: NormalTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.Normal"))
			, OperatorTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.Operator"))
			, KeywordTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.Keyword"))
			, StringTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.String"))
			, NumberTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.Number"))
			, CommentTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.Comment"))
			, PreProcessorKeywordTextStyle(FLuaCodeEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.CPP.PreProcessorKeyword"))
		{
		}

		FSyntaxTextStyle(const FTextBlockStyle& InNormalTextStyle, const FTextBlockStyle& InOperatorTextStyle, const FTextBlockStyle& InKeywordTextStyle, const FTextBlockStyle& InStringTextStyle, const FTextBlockStyle& InNumberTextStyle, const FTextBlockStyle& InCommentTextStyle, const FTextBlockStyle& InPreProcessorKeywordTextStyle)
			: NormalTextStyle(InNormalTextStyle)
			, OperatorTextStyle(InOperatorTextStyle)
			, KeywordTextStyle(InKeywordTextStyle)
			, StringTextStyle(InStringTextStyle)
			, NumberTextStyle(InNumberTextStyle)
			, CommentTextStyle(InCommentTextStyle)
			, PreProcessorKeywordTextStyle(InPreProcessorKeywordTextStyle)
		{
		}

		FTextBlockStyle NormalTextStyle;
		FTextBlockStyle OperatorTextStyle;
		FTextBlockStyle KeywordTextStyle;
		FTextBlockStyle StringTextStyle;
		FTextBlockStyle NumberTextStyle;
		FTextBlockStyle CommentTextStyle;
		FTextBlockStyle PreProcessorKeywordTextStyle;
	};

	static TSharedRef< FLuaRichTextSyntaxHighlighterTextLayoutMarshaller > Create(const FSyntaxTextStyle& InSyntaxTextStyle);

	virtual ~FLuaRichTextSyntaxHighlighterTextLayoutMarshaller();

protected:

	virtual void ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<ISyntaxTokenizer::FTokenizedLine> TokenizedLines) override;

	FLuaRichTextSyntaxHighlighterTextLayoutMarshaller(TSharedPtr< ISyntaxTokenizer > InTokenizer, const FSyntaxTextStyle& InSyntaxTextStyle);

	/** Styles used to display the text */
	FSyntaxTextStyle SyntaxTextStyle;

	/** String representing tabs */
	FString TabString;
};
