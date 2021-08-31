import React, {useState} from "react";

import './styles.scss'
import Frame from "../../components/SkyrimFrame";

import * as ru from '../../locales/ru.json';
import * as en from '../../locales/en.json'
import SkyrimButton from "../../components/SkyrimButton";
import SkyrimHint from "../../components/SkyrimHint";
import LoginForm from "./LoginForm";


const LoginPage = props => {
    const locale = props.locale ? props.locale : en
    const [isGithubHintOpened, setGithubHintOpened] = useState(false)
    const [isPatreonHintOpened, setPatreonHintOpened] = useState(false)
    const [isRegister, setRegister] = useState(false)
    return (
        <div className={'login'} >
            <div className={'login-form'}>
                <div className={'login-form--content'}>
                    <div className={'login-form--content_header'}>
                        {isRegister ? locale.LOGIN.HEADER_TEXT_REGISTER : locale.LOGIN.HEADER_TEXT_LOGIN}
                    </div>
                    <div className={'login-form--content_social'}>
                        <a href={'https://github.com/skyrim-multiplayer/skymp'}
                           target={'_blank'}
                           className={'login-form--content_social__link'}
                           onMouseOver={() => {
                               setGithubHintOpened(true)}
                           }
                           onMouseOut={() => setGithubHintOpened(false)}
                        >
                            <SkyrimHint
                                text={locale.LOGIN.GITHUB_HINT}
                                isOpened={isGithubHintOpened}
                                left={true}
                            />
                            <img src={require('../../img/github.svg').default}  alt={'Fork me on Github'} />
                        </a>
                        <a href={'https://github.com/skyrim-multiplayer/skymp'}
                           target={'_blank'}
                           className={'login-form--content_social__link'}
                           onMouseOver={() => {
                               setPatreonHintOpened(true)}
                           }
                           onMouseOut={() => setPatreonHintOpened(false)}
                        >

                            <SkyrimHint
                                text={locale.LOGIN.PATREON_HINT}
                                isOpened={isPatreonHintOpened}
                            />
                            <img src={require('../../img/patreon.svg').default}  alt={'Become a Patron'} />
                        </a>
                    </div>
                    {
                        isRegister ?
                            '' :
                            <LoginForm locale={locale}/>
                    }
                </div>
                <Frame />
            </div>
        </div>
    )
}

export default LoginPage